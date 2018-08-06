const ejs = require('ejs');
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
}

ejs.renderFile(options.template, templateData).then(function(code, err) {
	console.log(code);
	fs.writeFile(options.out, code, (err) => {
  		if (err) throw err;
  		console.log('Code generated');
  	});
});

function generateVariableNames(prefix, n) {
	return Array.from(Array(n)).map(function(_, i) { return prefix + (i+1)});
}

function generateDecodedOrderTuples(n) {
  return Array.from(Array(n)).map(function(_, i) { return `amount${i+1}, sourceToken${i+1}, targetToken${i+1}, limit${i+1}`});	
}