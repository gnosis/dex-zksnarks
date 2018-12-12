#![allow(unused_imports)]
#![allow(unused_variables)]
extern crate bellman;
extern crate pairing;
extern crate rand;
extern crate ff;
use bellman::{Circuit, ConstraintSystem, SynthesisError, Variable, LinearCombination};
use pairing::{Engine};
use std::fs::File;
use std::io::{BufReader, BufRead};
use std::time::{Instant};

use ff::{
    Field,
    PrimeField,
};

struct ParsedLine<E: Engine> {
    index: usize,
    lc: LinearCombination<E>,
    debug: String,
    result: E::Fr,
}

struct Assignment<E: Engine> {
    variable: Variable,
    constrained: bool,
    value: E::Fr,
}

fn empty_line<E:Engine>(index: usize) -> ParsedLine<E> {
    ParsedLine::<E>{
        index:index, 
        lc: LinearCombination::zero(), 
        debug: "".to_string(), 
        result: E::Fr::zero(),
    }
}

fn parse_line<E: Engine>(line: &String, variables: &mut Vec<Assignment<E>>, negate: bool) -> ParsedLine<E> {
    let parts: Vec<&str> = line.split_whitespace().collect();
    let variable: usize = parts[0].parse().unwrap();
    let index: usize = parts[1].parse().unwrap();

    // Bellman doesn't support parsing negative values.
    let mut coefficient = E::Fr::from_str(parts[2].trim_left_matches("-")).unwrap();
    if negate {
        coefficient.negate();
    }
    if parts[2].starts_with("-") { 
        coefficient.negate();
    }
    variables[variable] = Assignment::<E>{
        variable: variables[variable].variable, 
        constrained: true, 
        value: variables[variable].value
    };
    let mut result = coefficient;
    result.mul_assign(&(variables[variable].value));
    ParsedLine::<E> {
        index: index,
        lc: LinearCombination::zero() + (coefficient, variables[variable].variable),
        debug: format!("({}*{:?})", coefficient, variables[variable].value),
        result: result,
    }
}

fn read_next_constraint<E: Engine>(file: &mut BufReader<File>, variables: &mut Vec<Assignment<E>>, negate: bool) -> Option<ParsedLine<E>> {
    let mut line = String::new();
    let len = file.read_line(&mut line).unwrap();
    if len > 0 {
        Some(parse_line(&line, variables, negate))
    } else {
        None
    }
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

fn add_parsed_lines<E: Engine>(mut lhs: ParsedLine<E>,rhs: &ParsedLine<E>) -> ParsedLine<E> {
    assert!(lhs.index == rhs.index);
    lhs.result.add_assign(&rhs.result);
    ParsedLine {
        index: lhs.index,
        lc: lhs.lc + &rhs.lc,
        debug: lhs.debug + " + " + &rhs.debug,
        result: lhs.result,
    }
}

struct CircuitImpl<F: PrimeField> {
    primary_witness: Vec<F>,
    aux_witness: Vec<F>,
    qap_a: String,
    qap_b: String,
    qap_c: String,
}

impl<'a, E: Engine> Circuit<E> for &'a CircuitImpl<E::Fr> {
    fn synthesize<CS: ConstraintSystem<E>>(self, cs: &mut CS) -> Result<(), SynthesisError> {
        let now = Instant::now();

        println!("  {} Reading Witness", now.elapsed().as_secs());
        let aux: Vec<Assignment<E>> = self.aux_witness.clone().into_iter()
            .map(|value| Assignment {
                variable: cs.alloc(|| "aux", || Ok(value)).unwrap(), 
                constrained: false, 
                value: value
            }).collect();
        let primary: Vec<Assignment<E>> = self.primary_witness.clone().into_iter()
            .map(|value| Assignment {
                variable: cs.alloc_input(|| "primary", || Ok(value)).unwrap(), 
                constrained: false, 
                value: value
            }).collect();

        let mut vars = vec![
            Assignment::<E>{
                variable: CS::one(), 
                constrained: false, 
                value: E::Fr::one()
            }];
        vars.extend(aux);
        vars.extend(primary);

        println!("  {} Reading QAP", now.elapsed().as_secs());
        {
            let mut qap_a_file = BufReader::new(File::open(&self.qap_a)?);
            let mut qap_b_file = BufReader::new(File::open(&self.qap_b)?);
            let mut qap_c_file = BufReader::new(File::open(&self.qap_c)?);

            let mut current_a_lc = empty_line(0);
            let mut next_a_lc = read_next_constraint::<E>(&mut qap_a_file, &mut vars, false);

            let mut current_b_lc = empty_line(0);
            let mut next_b_lc = read_next_constraint::<E>(&mut qap_b_file, &mut vars, false);

            let mut current_c_lc = empty_line(0);
            let mut next_c_lc = read_next_constraint::<E>(&mut qap_c_file, &mut vars, true);

            while next_a_lc.is_some() || next_b_lc.is_some() || next_c_lc.is_some() {
                while next_a_lc.is_some() {
                    let mut advance = false;
                    {
                        let next = next_a_lc.as_ref().unwrap();
                        if next.index < current_a_lc.index {
                            println!("[WARNING] found term with index {} after index {}. This term will be ignored.", next.index, current_a_lc.index);
                            advance = true
                        } else if next.index == current_a_lc.index {
                            current_a_lc = add_parsed_lines::<E>(current_a_lc, next);
                            advance = true;
                        }
                    }
                    if advance {
                        next_a_lc = read_next_constraint::<E>(&mut qap_a_file, &mut vars, false);
                    } else {
                        break;
                    }
                }

                while next_b_lc.is_some() {
                    let mut advance = false;
                    {
                        let next = next_b_lc.as_ref().unwrap();
                        if next.index < current_b_lc.index {
                            println!("[WARNING] found term with index {} after index {}. This term will be ignored.", next.index, current_b_lc.index);
                            advance = true
                        } else if next.index == current_b_lc.index {
                            current_b_lc.lc = current_b_lc.lc + &next.lc;
                            current_b_lc.debug = current_b_lc.debug + " + " + &next.debug;
                            current_b_lc.result.add_assign(&next.result);
                            advance = true;
                        }
                    }
                    if advance {
                        next_b_lc = read_next_constraint::<E>(&mut qap_b_file, &mut vars, false);
                    } else {
                        break;
                    }
                }

                while next_c_lc.is_some() {
                    let mut advance = false;
                    {
                        let next = next_c_lc.as_ref().unwrap();
                        if next.index < current_c_lc.index {
                            println!("[WARNING] found term with index {} after index {}. This term will be ignored.", next.index, current_c_lc.index);
                            advance = true
                        } else if next.index == current_c_lc.index {
                            current_c_lc.lc = current_c_lc.lc + &next.lc;
                            current_c_lc.debug = current_c_lc.debug + " + " + &next.debug;
                            current_c_lc.result.add_assign(&next.result);
                            advance = true;
                        }
                    }
                    if advance {
                        next_c_lc = read_next_constraint::<E>(&mut qap_c_file, &mut vars, true);
                    } else {
                        break;
                    }
                }

                assert!(current_a_lc.index == current_b_lc.index);
                assert!(current_a_lc.index == current_c_lc.index);
                //println!("Constraint: {} * {} = {}", current_a_lc.debug, current_b_lc.debug, current_c_lc.debug);

                cs.enforce(
                    || current_a_lc.index.to_string(),
                    |lc| lc + &current_a_lc.lc,
                    |lc| lc + &current_b_lc.lc,
                    |lc| lc + &current_c_lc.lc,
                );

                current_a_lc.result.mul_assign(&current_b_lc.result);
                assert!(current_a_lc.result == current_c_lc.result, 
                format!("constraint {} failed: {} * {} != {}", current_a_lc.index, current_a_lc.debug, current_b_lc.debug, current_c_lc.debug));

                current_a_lc = empty_line(current_a_lc.index + 1);
                current_b_lc = empty_line(current_b_lc.index + 1);
                current_c_lc = empty_line(current_c_lc.index + 1);
            }
        }

        /*
         * For some reason, pepper can include variables in the witness that are not constrained.
         * Bellmann asserts that this is not the case. We could clean up the witness from unbound
         * variables and shift all constraints appropriately but adding dummy constraints seems to
         * be the easier option for now.
         */
        for var in vars.iter() {
            if !var.constrained {
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

    use pairing::bn256::{Bn256, Fr};
    use rand::thread_rng;
    use std::marker::PhantomData;

    use bellman::groth16::{
        create_random_proof, generate_random_parameters, prepare_verifying_key, verify_proof, Proof,
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

    let file = File::open(&args[1]).unwrap();
    match verify_proof(&pvk, &proof, &primary) {
        Ok(v) => assert!(v),
        Err(e) => println!("error: {:?}", e)
    }
    println!("{} Done", now.elapsed().as_secs());
}   