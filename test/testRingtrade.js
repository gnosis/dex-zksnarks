var assert = require('assert');
import { exec } from 'child_process';

async function sh(cmd) {
  return new Promise(function (resolve, reject) {
    exec(cmd, (err, stdout, stderr) => {
      if (err) {
        reject(err);
      } else {
        resolve({ stdout, stderr });
      }
    });
  });
}

describe('Ringrade', function() {
	it('should succeed if solution is valid', function() {
		sh("/home/zokrates/ZoKrates/target/release/zokrates -a 10 0 1 100000 10 1 2 500000 1000 2 0 1 1 10 5 100000 5000000").then(function() {
			assert(true);
		}, function(error) {
			asssert(false, error);
		});
    });
});