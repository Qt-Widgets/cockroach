/* 
 * File:   structs.h
 * Author: Ivan
 *
 * Created on 14 Август 2012 г., 10:50
 */

#ifndef STRUCTS_H
#define	STRUCTS_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef unsigned long long u64;
typedef long long          i64;

typedef unsigned int u32;
typedef int          i32;

typedef unsigned int u8;
typedef int          i8;

typedef double f64;
typedef float  f32;
    
#pragma pack (4)
    struct fap_prs {
        u8 id;
        u8 enabled;
        u8 symbol_locked;
        u8 locked;
        u8 reset;
        u8 noise_ready;
        i32 delay;
        i32 freq_offset;
        i32 freq_nominal;
        i32 freq_step;
        u32 range;
        u32 measure_index;
        f32 k0;
        f32 k1;
        f32 z0;
        f32 z1;
        f32 s;
        f32 c;
        f32 power;
        f32 power_locked;
        f32 noise;
        f32 noise_tmp;
        f32 level;
        u32 phase;
        f64 measure;
        u32 mseconds;
        u32 notused;
    };

#pragma pack (4)
    struct fap_freq {
        u8 id;
        u8 enabled;
        u8 current_locked;
        u8 locked;
        u8 dropped;
        u8 reset;
        f64 freq_offset;
        f64 freq_nominal;
        i32 drop_timeout;
        i32 lock_timeout;
        f32 k0;
        f32 k1;
        f32 z0;
        f32 z1;
        f32 s;
        f32 c;
        f32 si;
        f32 ci;
        f32 power;
        f32 level;
        f32 snr;
        u32 measure_index;
        f32 measure;
        u32 mseconds;
        f64 measure_phase;
        f64 phase_nominal;
    };

#pragma pack (4)
    struct chnl_inf {
        u8 id;
        u8 notused;
        u64 inf;
    };
    

#ifdef	__cplusplus
}
#endif

#endif	/* STRUCTS_H */

