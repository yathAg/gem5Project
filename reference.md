# Reference

## compile command

```console
scons-3 USE_HDF5=0 -j `nproc` ./build/ECE565-X86/gem5.opt
```

## Build command

```console
./build/ECE565-X86/gem5.opt --debug-flags=CacheAll --stats-file=compress.txt configs/spec/spec_se.py --cpu-type=O3CPU --maxinsts=10000000 --l1i_size=64kB --caches --l2cache --l2_size=4MB --cacheline_size 64 -b mcf
```

## Spec 2017 benchmarks

| Paper Benchmark | 2017 Benchmark |
|------------------|----------------|
| bzip             |  xz            |
| gcc              |  gcc           |
| mcf              |  mcf           |
| twolf            |  deepseng      |
| ammp             |  lbm           |
| applu            |  wrf           |
| equake           |  cam4          |
| swim             |  imagick_s     |

## System parameters

L1:

- [x] l1d_size :  64KB
– [x] l1i_size :  64KB (command line)
- [x] assoc : 2
- [x] replacement_policy : LRUP
- [x] data_latency : 2
- [x] entry_size : 64

L2:

- [x] l2_size : 4MB (command line)
- [x] Associativity : 8
- [x] replacement_policy : LRUP
- [x] response_latency : 20
- [ ] Line size : 64 byte

Memory:

- [ ] mem-size = 4GB
- [ ] 400 cycle access time and infinite bandwidth to set
- [ ] How to setup DRAM access time

CPU:

- [x] O3CPU :5 stage out of order (command line)
- [X] params.numROBEntries : 64 (updated in BaseO3.py)
- [ ] Branch predictor: TournamentBP
- [ ] Indirect branch predictor : SimpleIndirectPredictor 2 WAY
- [ ] RASSize : 16
