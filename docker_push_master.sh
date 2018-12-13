#!/bin/bash
echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin

docker tag dex_zokrates gnosispm/dex_zokrates:staging
docker push gnosispm/dex_zokrates:staging
docker tag dex_pepper gnosispm/dex_pepper:staging
docker push gnosispm/dex_pepper:staging
docker tag dex_bellman gnosispm/dex_bellman:staging
docker push gnosispm/dex_bellman:staging