extern crate bellman;
extern crate pairing;
extern crate rand;
extern crate ff;
extern crate rayon;
extern crate multimap;
use multimap::MultiMap;
use rayon::prelude::*;
use bellman::{Circuit, ConstraintSystem, SynthesisError, Variable, LinearCombination};
use pairing::{Engine};
use std::fs::File;
use std::io::{BufReader, BufRead};
use std::time::{Instant};
use std::sync::atomic::{AtomicBool, Ordering};

use ff::{
    Field,
    PrimeField,
};

struct Assignment {
    variable: Variable,
    constrained: AtomicBool,
}

fn parse_witness<F: PrimeField>(filename: &String) -> Vec<F> {
    let file = File::open(&filename).unwrap();
    let mut result = Vec::new();
    for line in BufReader::new(file).lines() {
        for var in line.expect("Couldn't parse line").split_whitespace() {
            result.push(F::from_str(var).unwrap());
        }
    }
    result
}

struct CircuitImpl<F: PrimeField> {
    primary_witness: Vec<F>,
    aux_witness: Vec<F>,
    qap_a: String,
    qap_b: String,
    qap_c: String,
}

#[derive(Copy, Clone)]
enum Matrix {
    A, B, C
}
struct LinearTerm<E: Engine> {
    variable: usize,
    coefficient: E::Fr,
    matrix: Matrix,
}

fn read_linear_terms_from_file<E: Engine>(filename: &String, matrix: Matrix) -> Vec<(usize, LinearTerm<E>)> {
    let file = File::open(&filename).expect("no such file");
    let buf = BufReader::new(file);
    buf.lines()
        .map(|l| l.expect("Could not parse line"))
        .collect::<Vec<String>>()
        .into_par_iter()
        .map(|line| {
            let parts: Vec<&str> = line.split_whitespace().collect();
            let variable: usize = parts[0].parse().unwrap();
            let index: usize = parts[1].parse().unwrap();

            // Bellman doesn't support parsing negative values.
            let mut coefficient = E::Fr::from_str(parts[2].trim_left_matches("-")).unwrap();
            if parts[2].starts_with("-") { 
                coefficient.negate();
            }
            (index, LinearTerm { variable, coefficient, matrix})
        }).collect()
}

impl<'a, E: Engine> Circuit<E> for &'a CircuitImpl<E::Fr> {
    fn synthesize<CS: ConstraintSystem<E>>(self, cs: &mut CS) -> Result<(), SynthesisError> {
        let now = Instant::now();

        println!("  {} Reading Witness", now.elapsed().as_secs());
        let aux: Vec<Assignment> = self.aux_witness.clone().into_iter()
            .map(|value| Assignment {
                variable: cs.alloc(|| "aux", || Ok(value)).unwrap(), 
                constrained: AtomicBool::new(false),
            }).collect();
        let primary: Vec<Assignment> = self.primary_witness.clone().into_iter()
            .map(|value| Assignment {
                variable: cs.alloc_input(|| "primary", || Ok(value)).unwrap(), 
                constrained: AtomicBool::new(false),
            }).collect();

        let mut vars = vec![
            Assignment {
                variable: CS::one(), 
                constrained: AtomicBool::new(false), 
            }];
        vars.extend(aux);
        vars.extend(primary);

        println!("  {} Reading QAP", now.elapsed().as_secs());

        // Map
        let mut mapped = read_linear_terms_from_file::<E>(&self.qap_a, Matrix::A);
        println!("    {} A done", now.elapsed().as_secs());
        mapped.extend(read_linear_terms_from_file(&self.qap_b, Matrix::B));
        println!("    {} B done", now.elapsed().as_secs());
        mapped.extend(read_linear_terms_from_file(&self.qap_c, Matrix::C));
        println!("    {} C done", now.elapsed().as_secs());

        // Group by index
        let grouped = mapped.into_iter().collect::<MultiMap<_,_>>()
            .into_iter().collect::<Vec<_>>();
        println!("    {} grouped", now.elapsed().as_secs());

        // Reduce by key
        let mut linear_combinations: Vec<(usize, LinearCombination<E>, LinearCombination<E>, LinearCombination<E>)> = grouped.into_par_iter()
            .map(|kv| {
                let mut lc_a = LinearCombination::<E>::zero();
                let mut lc_b = LinearCombination::<E>::zero();
                let mut lc_c = LinearCombination::<E>::zero();
                for linear_term in kv.1 {
                    match linear_term.matrix {
                        Matrix::A => lc_a = lc_a + (linear_term.coefficient, vars[linear_term.variable].variable),
                        Matrix::B => lc_b = lc_b + (linear_term.coefficient, vars[linear_term.variable].variable),
                        Matrix::C => lc_c = lc_c - (linear_term.coefficient, vars[linear_term.variable].variable), 
                    }
                    vars[linear_term.variable].constrained.store(true, Ordering::Relaxed);
                }
                (kv.0, lc_a, lc_b, lc_c)
            }).collect();
        linear_combinations.sort_by(|lhs, rhs| lhs.0.cmp(&rhs.0));

        println!("    {} reduced", now.elapsed().as_secs());
        for linear_combination in linear_combinations {
            cs.enforce(
                || linear_combination.0.to_string(),
                |lc| lc + &linear_combination.1,
                |lc| lc + &linear_combination.2,
                |lc| lc + &linear_combination.3,
            );
        }

        /*
         * For some reason, pepper can include variables in the witness that are not constrained.
         * Bellmann asserts that this is not the case. We could clean up the witness from unbound
         * variables and shift all constraints appropriately but adding dummy constraints seems to
         * be the easier option for now.
         */
        for var in vars.iter() {
            if !var.constrained.load(Ordering::Relaxed) {
                println!("[Warning] Unconstrained variable {:?}", var.variable);
                cs.enforce(
                    || format!("unused var {:?}", var.variable),
                    |lc| lc + var.variable,
                    |lc| lc + CS::one(),
                    |lc| lc + var.variable,
                );
            }
        }
        
        println!("  {} Done Reading", now.elapsed().as_secs());
        Ok(())
    }
}

fn print_usage(args: Vec<String>) {
    println!("
Program to create a zkSnark proof for an R1CS generated by the pepper framework.
It requires the R1CS (A, B, C matrices) as well as assignments for primary and auxiliary input.

The witnesses are consumed from a space separated list of field elements.
The matrices are consumed as files where each line corresponds to a nonzero entry in the corresponding QAP matrix. 
E.g. a line `96 5 2` means the 5th constraint contains the LinearTerm (witness_96 * 2).

The index of a primary input i_n is |aux| + n. The entire witness is prepended with 1 in 0th position.

By applying the witness, the matrices are expected to satisfy (A * B) + C = 0

Usage:
    {} <primary witness> <aux witness> <A matrix> <B matrix> <C matrix>", args[0])
}

fn main() {
    use std::env;
    let args: Vec<String> = env::args().collect();
    if args.len() != 6 {
        print_usage(args);
        ::std::process::exit(1);
    }

    use pairing::bn256::{Bn256};
    use rand::thread_rng;

    use bellman::groth16::{
        create_random_proof, generate_random_parameters, prepare_verifying_key, verify_proof,
    };

    let rng = &mut thread_rng();

    let primary = parse_witness(&args[1]);
    let aux = parse_witness(&args[2]);
    let c = CircuitImpl {
        primary_witness: primary.clone(),
        aux_witness: aux,
        qap_a: args[3].clone(),
        qap_b: args[4].clone(),
        qap_c: args[5].clone(),
    };

    let now = Instant::now();
    println!("{} start generate_random_parameters", now.elapsed().as_secs());
    let params = {
        generate_random_parameters::<Bn256, _, _>(&c, rng).unwrap()
    };
    
    println!("{} start prepare_verifying_key", now.elapsed().as_secs());
    let pvk = prepare_verifying_key(&params.vk);
    // Create a groth16 proof with our parameters.
    println!("{} start create_random_proof", now.elapsed().as_secs());
    let proof = create_random_proof(&c, &params, rng).unwrap();

    println!("{} start verify_proof", now.elapsed().as_secs());

    match verify_proof(&pvk, &proof, &primary) {
        Ok(v) => assert!(v),
        Err(e) => println!("error: {:?}", e)
    }
    println!("{} Done", now.elapsed().as_secs());
}   