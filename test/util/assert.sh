PASS='\033[0;32m[\xE2\x9C\x94]\033[0m'
FAIL='\033[0;31m[\xE2\x9D\x8C]\033[0m'

FAILED=0

function checkResponse() {
	if [ $? -eq 0 ]; then 
		echo -e "${PASS}"
	else 
		echo -e "${FAIL}"
		FAILED=1
	fi
}

function checkResponseInverse() {
	if [ $? -eq 0 ]; then 
		echo -e "${FAIL}"
		FAILED=1
	else 
		echo -e "${PASS}"
	fi
}

function assertAllPass() {
	if ((FAILED)); then
		echo "At least one test Failed"
		exit 1
	fi
}