# Reference

## Build command

```console
./build/ECE565-X86/gem5.opt configs/spec/spec_se.py --cpu-type=O3CPU --maxinsts=10000000 --l1d_size=64kB --l1i_size=64kB --caches --l2cache   --cacheline_size=64  -b gcc
```

## Spec 2017 benchmarks

- [ ] gcc
- [ ] mcf
- [ ] xz
- [ ] deepsjeng
- [ ] wrf
- [x] cam4
- [x] imagick
- [x] nab

## System parameters

L1:

- l1d_size :  64KB
– l1i_size :  64KB (default value is 32KB)
- Associativity : 2 (same as default)
- Need to check for LRU
- Need to check 2 cycle access time
- Line size : 64 byte

L2:

- l2_size : set it to 4MB
- Associativity : 2 (same as default)
- Need to check for LRU
- Need to check 2 cycle access time
- Line size : 64 byte

L3:

- num-l3caches : set it to 0 (diabling l3 cache)
- l3_size = 0MB (disabling l3)

Memory:

- mem-size = 4GB
- 400 cycle access time and infinite bandwidth to set
- How to setup DRAM access time

CPU:

- O3CPU ( 5 stage out of order)
- params.numROBEntries to set to 64 (In BaseO3.py , need to updated numROBEntries = Param.Unsigned(192, "Number of reorder buffer entries") to 64)
- Need to check YAGS branch predictor
- 64 entry indirect branch predictor
- 64 entry RAS
