## Bvox
### Header pattern
```c
u8 version @ 0x00;
u32 chunk_res @ 0x04;
u32 chunk_size @ 0x08;
bool run_length_encoded @ 0x0C;
bool morton_encoded @ 0x0D;
```
### Palette
Coming soon.
### Data Format
Each Voxel is an `u8`, which is a color index into the palette. Currently only `0` or `1` which indicates the voxel is either empty or filled.

## Bsvo
### Header pattern
```c
u8 version @ 0x00;
u8 max_depth @ 0x04;
u32 root_res @ 0x08;
bool run_length_encoded @ 0x0C;
```
### Palette
Coming soon.
### SvoNode Format
```c
u32 data @ 0x00;
u8 child_mask @ 0x04;
```