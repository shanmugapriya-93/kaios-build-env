#!/bin/sh

ragel -C -L -G2 EcrioSDPParse.rl
ragel -C -L -G2 EcrioSDPParseHeaderA.rl
ragel -C -L -G2 EcrioSDPParseHeaderA_accept-types.rl
ragel -C -L -G2 EcrioSDPParseHeaderA_AMR_parameters.rl
ragel -C -L -G2 EcrioSDPParseHeaderA_DTMF_parameters.rl
ragel -C -L -G2 EcrioSDPParseHeaderA_file-date.rl
ragel -C -L -G2 EcrioSDPParseHeaderA_file-range.rl
ragel -C -L -G2 EcrioSDPParseHeaderA_file-selector.rl
ragel -C -L -G2 EcrioSDPParseHeaderA_fmtp.rl
ragel -C -L -G2 EcrioSDPParseHeaderA_rtpmap.rl
ragel -C -L -G2 EcrioSDPParseHeaderA_precond_current.rl
ragel -C -L -G2 EcrioSDPParseHeaderA_precond_desired.rl
ragel -C -L -G2 EcrioSDPParseHeaderB.rl
ragel -C -L -G2 EcrioSDPParseHeaderC.rl
ragel -C -L -G2 EcrioSDPParseHeaderM.rl
ragel -C -L -G2 EcrioSDPParseHeaderO.rl
