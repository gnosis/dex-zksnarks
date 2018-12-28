# Running pepper with snarks from this repo

We will run our snarks inside a Docker container with a pre-installed version of pepper. 

From inside this folder run the following commands to get submodules, build and launch the docker container:

```sh
git submodule update --init --recursive
docker build -t dex_pepper .
docker run -v $PWD:/home/docker/dex-zksnarks -ti dex_pepper
```

This will give you a shell inside the docker container. 

The `-v $PWD:/home/docker/dex-zksnarks` argument will make it so that the contents of this folder (on your host-machine) are synced with the contents of `~/dex-zksnarks` inside the docker container.
There is also a script running in the background of your docker container that will watch for any changes to the `Makefile` and automatically call `make all` when they are saved.

The `Makefile` has three main tasks:
1. Link the snark source code (`apps/*.c`) into the pepper's `apps/` folder. 
2. Link external computation scripts (`exo_compute`), such as the private input provider, into pepper `bin/` folder.
3. Compile the `ext_gadget` scripts into pepper's `bin/` folder.

If you add any components to this repo, make sure to also adjust the `Makefile` so that your components will be available in the pepper directory without any manual commands. 
The `Makefile` is also executed when the container is first built.


## Running `hash_transform.c`

To compile, prove and verify the `hash_transform.c` example, specify the number of orders inside `apps/hash_transform.c` (on your host machine):

```C
#define ORDERS 2 // replace with any number
```

Changes will be automatically synced with your docker container.

Order data is provided as a private input by calling `./exo_compute/private_input.py`. 
The public input to the snark is the `sha3` hash of all your orders hashed together as a list. 
For now, this value can be computed by calling:

```sh
python3 order_transform/generate.py <num_orders>
```

The script outputs the Pedersen hash (an EC Point), as well as the matching `sha3` hash for the provided number of orders (as a tuple with two entries, since 256bit is larger than a single field element).

Write the `sha3` hash into the file that pepper will read as public input by calling:

```sh
echo <sha_hash[0]> > prover_verifier_shared/transform.inputs
echo <sha_hash[1]> >> prover_verifier_shared/transform.inputs
```

inside the Docker container.
Now you can compile, run and verify the snark program with:

```sh
./pepper_compile_and_setup_P.sh hash_transform
./pepper_compile_and_setup_V.sh hash_transform transform.vkey transform.pkey
./bin/pepper_prover_hash_transform prove transform.pkey transform.inputs transform.outputs transform.proof
./bin/pepper_verifier_hash_transform verify transform.vkey transform.inputs transform.outputs transform.proof
```

You should see `VERIFICATION SUCCESSFUL` printed at the end. 
If you inspect `prover_verifier_shared/transform.outputs` you should also see the same Pedersen hash (x and y coordinate) that the `order_transform/generate.py` script created.
