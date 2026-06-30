# Boruvka MST Benchmark Results

Speedup is relative to `serial_full`, computed from the median run time over all runs (time in microseconds).

## 2d-2e20.sym 

MST weight: `2806989831`

| Version | Median time (µs) | Speedup vs serial_full |
| --- | ---: | ---: |
| serial_full | 292,966 | 1.00× |
| serial_half | 239,040 | 1.23× |
| serial_split | 272,124 | 1.08× |
| omp_half | 61,242 | 4.78× |

## USA-road-d.NY

MST weight: `262559014`

| Version | Median time (µs) | Speedup vs serial_full |
| --- | ---: | ---: |
| serial_full | 49,892 | 1.00× |
| serial_half | 42,602 | 1.17× |
| serial_split | 46,956 | 1.06× |
| omp_half | 11,314 | 4.41× |
 
## USA-road-d.USA

MST weight: `60109136592`

| Version | Median time (µs) | Speedup vs serial_full |
| --- | ---: | ---: |
| serial_full | 4,922,728 | 1.00× |
| serial_half | 4,303,707 | 1.14× |
| serial_split | 4,687,270 | 1.05× |
| omp_half | 1,235,838 | 3.98× |

## internet

MST weight: `951569`

| Version | Median time (µs) | Speedup vs serial_full |
| --- | ---: | ---: |
| serial_full | 14,949 | 1.00× |
| serial_half | 12,368 | 1.21× |
| serial_split | 13,636 | 1.10× |
| omp_half | 3,872 | 3.86× |

## kron_g500-logn21

MST weight: `1544012`

| Version | Median time (µs) | Speedup vs serial_full |
| --- | ---: | ---: |
| serial_full | 1,001,210 | 1.00× |
| serial_half | 1,027,956 | 0.97× |
| serial_split | 1,005,093 | 1.00× |
| omp_half | 256,009 | 3.91× |

## r4-2e23.sym

MST weight: `12284371526`

| Version | Median time (µs) | Speedup vs serial_full |
| --- | ---: | ---: |
| serial_full | 7,020,549 | 1.00× |
| serial_half | 7,151,452 | 0.98× |
| serial_split | 7,487,150 | 0.94× |
| omp_half | 2,135,417 | 3.29× |

## rmat16.sym

MST weight: `693935344`

| Version | Median time (µs) | Speedup vs serial_full |
| --- | ---: | ---: |
| serial_full | 27,000 | 1.00× |
| serial_half | 25,620 | 1.05× |
| serial_split | 26,290 | 1.03× |
| omp_half | 5,676 | 4.76× |

## rmat22.sym

MST weight: `3007276592424`

| Version | Median time (µs) | Speedup vs serial_full |
| --- | ---: | ---: |
| serial_full | 3,111,720 | 1.00× |
| serial_half | 2,890,332 | 1.08× |
| serial_split | 2,973,674 | 1.05× |
| omp_half | 688,020 | 4.52× |

