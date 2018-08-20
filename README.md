# dex-zksnarks
This repository contains a collection of tools and code to proof that a given solution for a decentralized multi-token batch auction as described in [our research paper](https://github.com/gnosis/dex-research/releases) fulfills certain fairness criterias. Please also refer to [this video](https://www.youtube.com/watch?v=r7WY0VUqfNU) for more context.

Given the following input
1. `n` sell orders where each order is a tuple of `(amount, sell token, buy token, limit price)`, read as a person being willing to sell `amount` units of `sell token` if they get at least `amount * limit price` of `buy token`.
2. `n` bits where `bit == 1` if the order was double signed in the batch auction. Orders which haven't been double signed are ignored in the constraints below.
3. `n` volumes, where `volume_i` indicates the `amount` of `order_i` that has been sold, ranging between `0` and `amount`.
4. `m-1` prices for `m` tokens that are traded in the batch. `token_i` in `1..m-1` denotes the units that traders receive in the reference token (thought of as `token_0`) for one unit of `token_i`. Prices are arbitrage free meaning that `price(a, b) == 1/price(b, a)`

The solver verifies the following constraints
1. For each order, `volume <= amount`
2. For each order, `volume > 0 => limit_price <= price(source token, target token)`
3. For each token, `sum(volume bought) == sum(volume sold)`
4. If `order_i` has a positive trading volume, then every `order_j` with a lower limit price should be completely fulfilled.

Given that these constraints are verified we consider the solution valid and fair. This is, we are only executing trades with low enough limits, we are not burning or creating tokens out of nowhere, and we are not executing more expensive orders before we have exhausted "better" ones . 

We expect to use the snark proof submitted on the ethereum main chain in case there is a dispute about the validity of a solution provided by the exchange operator or participants. 

## Running an example on a local machine

The easiest way to run the system locally is to verify an example ring trade using a prebuilt docker image.

From inside the repo run:

```
docker build -t dex-snark .
docker run -ti dex-snark /bin/bash
cd ZoKrates/target/release/
```

Then, to generate the ZoKrates code that verifies the constraints for `o` orders and `t` tokens (choosing 3 for a simple ring trade), run
```
node ~/dex-snark/codegen/generate.js \
  --template ~/dex-snark/codegen/templates/verify_constraints_bitwise_order_encoding.ejs \
  --out ~/dex-snark/batch_auction.code \
  -t 3 -o 3
```

and compile the generated output using ZoKrates:
```
./zokrates compile -i ~/dex-snark/batch_auction.code
./zokrates setup
```

To convert file containing human readible results of the optimization problem (json) into the more cryptic representation required by the snark program, run:

```
~/dex-snark/util/generate_input.py 3 3 253 ~/dex-snark/examples/ringtrade.json
```

It outputs the command that runs the verifier on the generated input file.
```
./zokrates compute-witness --interactive < data.input
./zokrates generate-proof
```

This returns the 8 points, which have to be posted on the main chain to proof the calculation has been done correctly.

## Input Encoding

The following section contains an overview of how we encode the inputs for the snark program.

### Numbers

Numbers are respresented as Integers shifted with `10**18`. This allows to encode numbers with up to 18 decimals (smallest unit for tokens following the ERC standard) and thus includes the smallest tradeable amount.

E.g. `0.1234 ==> int(0.1234 * 10**18) ==> 123400000000000000`

### Orders
There are two ways to represent orders:
1. As a single `uint` with `o < 2**253` (since `2**253 < p < 2**253` in ZoKrates finite field limits `max(uint)`)
2. Bitwise as 253 inputs per order, each being either `0` or `1`

We found 2. to be much more performant (1. already takes >100k circuits per order). We are therefore likely going to focus on that approach.

The 253 bits are allocated as follows (MSB --> LSB)
- Padding 3 bit
- Limit price (120 bits) - cf number encoding above
- Target token (5 bits - i.e. we support 32 = 2^5 tokens)
- Source token (5 bits)
- Amount (120 bits)

### Bitmap
If there are `n` orders the next `n` parameters are either `0` or `1` indicating whether the order was double signed by it's owner and should therefore be considered in the calculation.

### Volumes
The next `n` parameters are encoded numbers that denote how much of the corresponding order's offered sell volume has been fulfilled. Note, that `volume_i <= amount_i`.

### Prices
If there are `m` tokens, `m-1` encoded number, each indicating `price(token_i, token_0)`

There is also a utility tool to encode number and orders
```
 ~/dex-snark/util/encode.py number 1
 ~/dex-snark/util/encode.py order 10 0 1 5 # amount: 10, source token: 0, target token: 1, limit: 5
```
## Code Generation

Since zkSnarks cannot compute dynamic loops based on the size of the input, we need to generate the snark program with a fixed input size in mind. 
Code generation can be run via:

```
node ~/dex-snark/codegen/generate.js \
  --template <template> \
  --out <output> \
  -t <num tokens> -o <num orders>
```

This generates a `.code` file which serves as an input for the [ZoKrates](https://github.com/JacobEberhardt/ZoKrates) framework.

There are two main templates in the `codegen/templates` folder depending on whether you encode each order as a single `uint` or bitwise.
`<output>` should be in the `~/dex-snark folder` as it imports subprograms located in that folder.
It is possible to use a snark program with fewer orders/tokens than it has been generated for, by filling non-existent values with 0.

## Generating/Transforming inputs for the prover

To generate inputs for a snark program, we can either set all values (orders, volumes, prices) to `0`:

```
~/dex-snark/util/generate_input.py \
  <num_orders> \
  <num_tokens> \
  <bits per order> # (1 or 253 depending on type of order encoding)
```

This is a valid solution and can be used to benchmark compilation and execution times.

Or we can also convert a human readable json output in the form:

```
{
  
  "tokens": [
    "ETH", 
    ...
  ], 
  "pricesNew": {
    "ETH": 500, 
    ...
  }, 
  "refToken": "ETH",
  "sellOrders": [
    {
      "sellAmount": 10.0, 
      "sellToken": "ETH", 
      "buyToken": "GNO", 
      "limitRate": [
        [1.0, "ETH"], 
        [10.0, "GNO"]
      ],  
      "execSellAmount": 1.0
    },
    ...
  ]
}
```

into inputs using:

```
~/dex-snark/util/generate_input.py <num_orders> <num_tokens> <bits per order> <path to json file>
```

We can also check all constraints for a human reasonable json file using a python verifier (much faster than running it with Zokrates).

```
~/dex-snark/util/verify.py <path to json file>

```

## Unit Tests
From docker's `~/ZoKrates/target/release/`, run

```
~/dex-snark/test/ringtrade_counterexamples.sh
~/dex-snark/test/run_util_tests.sh
~/dex-snark/test/run_tests.sh
```
