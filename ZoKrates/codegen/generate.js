const ejs = require('ejs-promise');
const commandLineArgs = require('command-line-args')
const fs = require('fs')

const optionDefinitions = [
  { name: 'template', type: String},
  { name: 'out', type: String},
  { name: 'orders', alias: 'o', type: Number },
  { name: 'tokens', alias: 't', type: Number }
]
const options = commandLineArgs(optionDefinitions)
const templateData = {
	orders: options.orders,
	tokens: options.tokens,
	generateVariableNames,
	generateDecodedOrderTuples,
  generateTokenPairs,
  generateBitwiseVariables
}

ejs.renderFile(options.template, templateData, function(err, resultPromise) {
  resultPromise.noBuffer();
  resultPromise.outputStream.pipe(fs.createWriteStream(options.out));
});

function generateVariableNames(prefix, n) {
	return Array.from(Array(n)).map(function(_, i) { return prefix + (i+1)});
}

function generateDecodedOrderTuples(n) {
  return Array.from(Array(n)).map(function(_, i) { return `amount${i+1}, sourceToken${i+1}, targetToken${i+1}, limit${i+1}`});	
}

function generateTokenPairs(prefix, n) {
  var pairs = [];
  for(i=0; i < n; i++) {
    for(j=0; j < n; j++) {
      if (i == j) { 
        continue;
      }
      pairs.push({
        source: i, 
        target: j, 
        name: `${prefix}Token${i}Token${j}`});
    }
  }
  return pairs;
}

function generateBitwiseVariables(prefix, bits, n) {
  return {
    get: function(index) {
      let i = Math.floor(index / bits);
      let bit = index % bits;
      return `${prefix}${i+1}Bit${bit}`;
    },
    join: function(glue) {
      if (bits * n == 0) {
        return '';
      }
      var result = this.get(0)
      for (i=1; i < bits * n; i++) {
        result += glue + this.get(i);
      }
      return result;
    },
    slice: function(lower, upper) {
      result = [];
      for (i = lower; i < upper; i++) {
        result.push(this.get(i));
      }
      return result
    }
  }
}