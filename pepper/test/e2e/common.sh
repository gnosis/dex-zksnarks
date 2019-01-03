function runApp() {
    cd $PEPPER/pepper

    ./pepper_compile_and_setup_P.sh $1
    ./pepper_compile_and_setup_V.sh $1 $1.vkey $1.pkey

    ./bin/pepper_prover_$1 prove $1.pkey $1.inputs $1.outputs $1.proof
    ./bin/pepper_verifier_$1 verify $1.vkey $1.inputs $1.outputs $1.proof
    
    cd -
}