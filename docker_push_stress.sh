#!/bin/bash
echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin

docker tag dex_zokrates gnosispm/dex_zokrates:stress-test
docker push gnosispm/dex_zokrates:stress-test
docker tag dex_pepper gnosispm/dex_pepper:stress-test
docker push gnosispm/dex_pepper:stress-test