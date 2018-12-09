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

struct CircuitImpl {
    witness: String,
    qap_a: String,
    qap_b: String,
    qap_c: String,
}

fn parse_line<E: Engine>(line: &String, variables: &mut Vec<(Variable, bool, E::Fr)>, negate: bool) -> ParsedLine<E> {
    let parts: Vec<&str> = line.split_whitespace().collect();
    let row: usize = parts[0].parse().unwrap();
    let column: usize = parts[1].parse().unwrap();

    // Bellman doesn't support parsing negative values.
    let mut coefficient = E::Fr::from_str(parts[2].trim_left_matches("-")).unwrap();
    if negate {
        coefficient.negate();
    }
    if parts[2].starts_with("-") { 
        coefficient.negate();
    }
    variables[row] = (variables[row].0, true, variables[row].2);
    let mut result = coefficient;
    result.mul_assign(&(variables[row].2));
    ParsedLine::<E> {
        index: column,
        lc: LinearCombination::zero() + (coefficient, variables[row].0),
        debug: format!("({}*{:?})", coefficient, variables[row].2),
        result: result,
    }
}

fn read_next_constraint<E: Engine>(file: &mut BufReader<File>, variables: &mut Vec<(Variable, bool, E::Fr)>, negate: bool) -> Option<ParsedLine<E>> {
    let mut line = String::new();
    let len = file.read_line(&mut line).unwrap();
    if len > 0 {
        Some(parse_line(&line, variables, negate))
    } else {
        None
    }
}

// Implementation of our circuit.
impl<'a, E: Engine> Circuit<E> for &'a CircuitImpl {
    fn synthesize<CS: ConstraintSystem<E>>(self, cs: &mut CS) -> Result<(), SynthesisError> {
        
        let now = Instant::now();
        println!("  {} Reading Witness", now.elapsed().as_secs());
        let mut vars: Vec<(Variable, bool /* constrained */, E::Fr)> = Vec::new();
        vars.push((CS::one(), false, E::Fr::one()));
        {
            let file = File::open(&self.witness)?;
            for line in BufReader::new(file).lines() {
                for (i, var) in line.expect("Couldn't parse line").split_whitespace().enumerate() {
                    let value =  E::Fr::from_str(var).unwrap();
                    let var = cs.alloc(|| var, || Ok(value))?;
                    vars.push((var, false, value));
                }
            }
        }
        println!("  {} Reading QAP", now.elapsed().as_secs());
        {
            let mut qap_a_file = BufReader::new(File::open(&self.qap_a)?);
            let mut qap_b_file = BufReader::new(File::open(&self.qap_b)?);
            let mut qap_c_file = BufReader::new(File::open(&self.qap_c)?);

            let mut current_a_lc = ParsedLine::<E>{
                index:0 , 
                lc: LinearCombination::zero(), 
                debug: "".to_string(),
                result: E::Fr::zero(),
            };
            let mut next_a_lc = read_next_constraint::<E>(&mut qap_a_file, &mut vars, false);

            let mut current_b_lc = ParsedLine::<E>{
                index:0,
                lc: LinearCombination::zero(),
                debug: "".to_string(),
                result: E::Fr::zero()
            };
            let mut next_b_lc = read_next_constraint::<E>(&mut qap_b_file, &mut vars, false);

            let mut current_c_lc = ParsedLine::<E>{
                index:0 , 
                lc: LinearCombination::zero(), 
                debug: "".to_string(), 
                result: E::Fr::zero(),
            };
            let mut next_c_lc = read_next_constraint::<E>(&mut qap_c_file, &mut vars, true);

            while next_a_lc.is_some() || next_b_lc.is_some() || next_c_lc.is_some() {
                while next_a_lc.is_some() {
                    let mut advance = false;
                    {
                        let next = next_a_lc.as_ref().unwrap();
                        assert!(next.index >= current_a_lc.index);
                        if next.index == current_a_lc.index {
                            current_a_lc.lc = current_a_lc.lc + &next.lc;
                            current_a_lc.debug = current_a_lc.debug + " + " + &next.debug;
                            current_a_lc.result.add_assign(&next.result);
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
                        assert!(next.index >= current_b_lc.index);
                        if next.index == current_b_lc.index {
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
                        assert!(next.index >= current_c_lc.index);
                        if next.index == current_c_lc.index {
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

                current_a_lc = ParsedLine::<E>{
                    index:current_a_lc.index+1 , 
                    lc: LinearCombination::zero(), 
                    debug: "".to_string(), 
                    result: E::Fr::zero(),
                };
                current_b_lc = ParsedLine::<E>{
                    index:current_b_lc.index+1 ,
                    lc: LinearCombination::zero(),
                    debug: "".to_string(),
                    result: E::Fr::zero(),
                };
                current_c_lc = ParsedLine::<E>{
                    index:current_c_lc.index+1 ,
                    lc: LinearCombination::zero(),
                    debug: "".to_string(),
                    result: E::Fr::zero(),
                };
            }
        }

        /**
         * For some reason, pepper can include variables in the witness that are not constrained.
         * Bellmann asserts that this is not the case. We could clean up the witness from unbound
         * variables and shift all constraints appropriately but adding dummy constraints seems to
         * be the easier option for now.
         */
        for (i, item) in vars.iter().enumerate() {
            if !item.1 {
                println!("[Warning] Unconstrained variable {:?}", item.0);
                cs.enforce(
                    || format!("unused var {}", i),
                    |lc| lc + item.0,
                    |lc| lc + CS::one(),
                    |lc| lc + item.0,
                );
            }
        }

        println!("  {} Done Reading", now.elapsed().as_secs());
        Ok(())
    }
}

// Create some parameters, create a proof, and verify the proof.
fn main() {
    use pairing::bn256::{Bn256, Fr};
    use rand::thread_rng;
    use std::marker::PhantomData;

    use bellman::groth16::{
        create_random_proof, generate_random_parameters, prepare_verifying_key, verify_proof, Proof,
    };

    let rng = &mut thread_rng();
    let c = CircuitImpl {
        witness: "data/hash_transform.witness".to_string(),
        qap_a: "data/hash_transform.qap.matrix_a".to_string(),
        qap_b: "data/hash_transform.qap.matrix_b".to_string(),
        qap_c: "data/hash_transform.qap.matrix_c".to_string()
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
    match verify_proof(&pvk, &proof, &[]) {
        Ok(v) => assert!(v),
        Err(e) => println!("error: {:?}", e)
    }
    println!("{} Done", now.elapsed().as_secs());
}   