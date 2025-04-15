#!/bin/sh

ragel -C -L -G2 EcrioCPMParseXML.rl
ragel -C -L -G2 EcrioCPMParseMultipartMessage.rl

