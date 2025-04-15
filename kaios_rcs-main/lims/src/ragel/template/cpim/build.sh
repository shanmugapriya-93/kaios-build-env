#!/bin/sh

ragel -C -L -G2 EcrioCPIMParse.rl
ragel -C -L -G2 EcrioCPIMParseMessage.rl

