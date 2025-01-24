#!/bin/bash

mkdir -p ./previous
cp -rf ./tester/stm32tester/Core/MCTP/src ./tester/stm32tester/Core/MCTP/include ./previous
cp -rf ./src ./include ./tester/stm32tester/Core/MCTP/
