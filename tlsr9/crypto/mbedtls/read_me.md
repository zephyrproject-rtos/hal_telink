# MbedTLS HW acceleration for Telink B91 platform

## HW supported units
- random generator
- elliptic curves cryptographic unit
- AES unit

## Folder structure
- **read_me.md** - this document
- **mbedtls_config.h** - mbedtls configuration file
- **ecp_alt.h** - alternative configuration for elliptic curves (no differences from original library implementation)
- **internal** - platform implementation source folder
- **internal/common.h** - general include file (copied from mbedtls library without any modifications)
- **internal/ecp_alt_b91_backend.c** - mbedtls wrapper for B91 HW elliptic curves cryptographic unit
- **internal/ecp_alt_b91_backend_test.c** - test vectors and test procedure for elliptic curves cryptographic unit
- **internal/entropy_poll_alt.c** - mbedtls wrapper for B91 HW random unit
- **internal/multithread.c** - multithread exclusive access implementation for B91 HW cryptographic units (*fill functions depending on your RTOS*)
- **internal/multithread.h** - multithread exclusive access header for B91 HW cryptographic units
- **internal/test_utils.c** - test utilities implementation
- **internal/test_utils.c** - test utilities header
- **internal/compatibility** - this folder contains original library files with minor modifications to provide HW acceleration compilation out of original source tree

## Compatibility folder
These files are copied from original mbedtls library folder to make compilation out of mbedtls source tree (no modification of original mbedtls implementation).
Some minor changes are applied over original files to provide HW acceleration support.

The changes are the next:

    diff --git a/library/aes.c b/library/aes.c
    index a15a80924..ba636f43b 100644
    --- a/library/aes.c
    +++ b/library/aes.c
    @@ -23,7 +23,7 @@
      *  http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
      */
     
    -#include "common.h"
    +#include "../common.h"
     
     #if defined(MBEDTLS_AES_C)
     
    @@ -49,7 +49,10 @@
     #endif /* MBEDTLS_PLATFORM_C */
     #endif /* MBEDTLS_SELF_TEST */
     
    -#if !defined(MBEDTLS_AES_ALT)
    +#if defined(MBEDTLS_AES_ALT)
    +
    +#include "../multithread.h"
    +#include "aes.h"
     
     /* Parameter validation macros based on platform_util.h */
     #define AES_VALIDATE_RET( cond )    \
    @@ -557,6 +560,13 @@ int mbedtls_aes_setkey_enc( mbedtls_aes_context *ctx, const unsigned char *key,
         AES_VALIDATE_RET( ctx != NULL );
         AES_VALIDATE_RET( key != NULL );
     
    +    if( keybits == 128 )
    +    {
    +        ctx->nr = 10;
    +        memcpy( ctx->buf, key, 16 );
    +        return 0;
    +    }
    +
         switch( keybits )
         {
             case 128: ctx->nr = 10; break;
    @@ -675,6 +685,13 @@ int mbedtls_aes_setkey_dec( mbedtls_aes_context *ctx, const unsigned char *key,
         AES_VALIDATE_RET( ctx != NULL );
         AES_VALIDATE_RET( key != NULL );
     
    +    if( keybits == 128 )
    +    {
    +        ctx->nr = 10;
    +        memcpy( ctx->buf, key, 16 );
    +        return 0;
    +    }
    +
         mbedtls_aes_init( &cty );
     
     #if defined(MBEDTLS_PADLOCK_C) && defined(MBEDTLS_PADLOCK_ALIGN16)
    @@ -865,6 +882,13 @@ int mbedtls_internal_aes_encrypt( mbedtls_aes_context *ctx,
                                       const unsigned char input[16],
                                       unsigned char output[16] )
     {
    +    if ( ctx->nr == 10 ) {
    +        mbedtls_aes_lock();
    +        ( void ) aes_encrypt( ( unsigned char * )ctx->buf, ( unsigned char * )input, output );
    +        mbedtls_aes_unlock();
    +        return 0;
    +    }
    +
         int i;
         uint32_t *RK = ctx->rk;
         struct
    @@ -929,6 +953,13 @@ int mbedtls_internal_aes_decrypt( mbedtls_aes_context *ctx,
                                       const unsigned char input[16],
                                       unsigned char output[16] )
     {
    +    if ( ctx->nr == 10 ) {
    +        mbedtls_aes_lock();
    +        ( void ) aes_decrypt( ( unsigned char * )ctx->buf, ( unsigned char * )input, output );
    +        mbedtls_aes_unlock();
    +        return 0;
    +    }
    +
         int i;
         uint32_t *RK = ctx->rk;
         struct
    @@ -1453,9 +1484,9 @@ int mbedtls_aes_crypt_ctr( mbedtls_aes_context *ctx,
     }
     #endif /* MBEDTLS_CIPHER_MODE_CTR */
     
    -#endif /* !MBEDTLS_AES_ALT */
    +#endif /* MBEDTLS_AES_ALT */
     
    -#if defined(MBEDTLS_SELF_TEST)
    +#if 0 /* use (MBEDTLS_SELF_TEST) from library */
     /*
      * AES test vectors from:
      *
    diff --git a/library/ecp.c b/library/ecp.c
    index e8df7ff28..49300002d 100644
    --- a/library/ecp.c
    +++ b/library/ecp.c
    @@ -39,7 +39,7 @@
      *     <http://eprint.iacr.org/2004/342.pdf>
      */
     
    -#include "common.h"
    +#include "../common.h"
     
     /**
      * \brief Function level alternative implementation.
    @@ -82,7 +82,22 @@
     
     #include <string.h>
     
    -#if !defined(MBEDTLS_ECP_ALT)
    +#if defined(MBEDTLS_ECP_ALT)
    +
    +#include "pke.h"
    +
    +/* HW accelerator functionality */
    +extern int ecp_alt_b91_backend_check_pubkey( const mbedtls_ecp_group *grp, const mbedtls_ecp_point *pt );
    +extern int ecp_alt_b91_backend_mul( mbedtls_ecp_group *grp, mbedtls_ecp_point *R,
    +                                    const mbedtls_mpi *m, const mbedtls_ecp_point *P );
    +extern int ecp_alt_b91_backend_muladd( mbedtls_ecp_group *grp, mbedtls_ecp_point *R,
    +                                       const mbedtls_mpi *m, const mbedtls_ecp_point *P,
    +                                       const mbedtls_mpi *n, const mbedtls_ecp_point *Q );
    +/* self test functionality */
    +#if defined(MBEDTLS_SELF_TEST)
    +extern const int __ecp_alt_b91_skip_internal_self_tests;
    +extern int ecp_alt_b91_backend_test(int verbose);
    +#endif /* MBEDTLS_SELF_TEST */
     
     /* Parameter validation macros based on platform_util.h */
     #define ECP_VALIDATE_RET( cond )    \
    @@ -2527,6 +2542,9 @@ int mbedtls_ecp_mul_restartable( mbedtls_ecp_group *grp, mbedtls_ecp_point *R,
         if( f_rng == NULL )
             return( MBEDTLS_ERR_ECP_BAD_INPUT_DATA );
     
    +    if( GET_WORD_LEN( grp->pbits ) <= PKE_OPERAND_MAX_WORD_LEN )
    +        return( ecp_alt_b91_backend_mul( grp, R, m, P ) );
    +
         return( ecp_mul_restartable_internal( grp, R, m, P, f_rng, p_rng, rs_ctx ) );
     }
     
    @@ -2658,6 +2676,9 @@ int mbedtls_ecp_muladd_restartable(
         if( mbedtls_ecp_get_type( grp ) != MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS )
             return( MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE );
     
    +    if( GET_WORD_LEN( grp->pbits ) <= PKE_OPERAND_MAX_WORD_LEN )
    +        return( ecp_alt_b91_backend_muladd( grp, R, m, P, n, Q ) );
    +
         mbedtls_ecp_point_init( &mP );
     
         ECP_RS_ENTER( ma );
    @@ -2877,8 +2898,15 @@ int mbedtls_ecp_check_pubkey( const mbedtls_ecp_group *grp,
             return( ecp_check_pubkey_mx( grp, pt ) );
     #endif
     #if defined(MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED)
    -    if( mbedtls_ecp_get_type( grp ) == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS )
    -        return( ecp_check_pubkey_sw( grp, pt ) );
    +    if( GET_WORD_LEN( grp->pbits ) <= PKE_OPERAND_MAX_WORD_LEN ) {
    +
    +    }
    +    if( mbedtls_ecp_get_type( grp ) == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS ) {
    +        if( GET_WORD_LEN( grp->pbits ) <= PKE_OPERAND_MAX_WORD_LEN )
    +            return( ecp_alt_b91_backend_check_pubkey( grp, pt ) );
    +        else
    +            return( ecp_check_pubkey_sw( grp, pt ) );
    +    }
     #endif
         return( MBEDTLS_ERR_ECP_BAD_INPUT_DATA );
     }
    @@ -3326,6 +3354,9 @@ cleanup:
      */
     int mbedtls_ecp_self_test( int verbose )
     {
    +    if( __ecp_alt_b91_skip_internal_self_tests )
    +        return ecp_alt_b91_backend_test(verbose);
    +
         int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
         mbedtls_ecp_group grp;
         mbedtls_ecp_point R, P;
    diff --git a/library/ecp_curves.c b/library/ecp_curves.c
    index 421a067bb..46ae41e7f 100644
    --- a/library/ecp_curves.c
    +++ b/library/ecp_curves.c
    @@ -17,7 +17,7 @@
      *  limitations under the License.
      */
     
    -#include "common.h"
    +#include "../common.h"
     
     #if defined(MBEDTLS_ECP_C)
     
    @@ -30,7 +30,7 @@
     
     #include <string.h>
     
    -#if !defined(MBEDTLS_ECP_ALT)
    +#if defined(MBEDTLS_ECP_ALT)
     
     /* Parameter validation macros based on platform_util.h */
     #define ECP_VALIDATE_RET( cond )    \
    diff --git a/library/ecp_invasive.h b/library/ecp_invasive.h
    index 71c770275..bc4a53d34 100644
    --- a/library/ecp_invasive.h
    +++ b/library/ecp_invasive.h
    @@ -26,7 +26,7 @@
     #ifndef MBEDTLS_ECP_INVASIVE_H
     #define MBEDTLS_ECP_INVASIVE_H
     
    -#include "common.h"
    +#include "../common.h"
     #include "mbedtls/bignum.h"
     #include "mbedtls/ecp.h"

So patch is applied on original mbedtls source tree and result copied to **internal/compatibility** folder. Changes on original source tree are removed.
Files **ecp_curves.c**, **ecp.c** and **aes.h** are renamed to alternative implementation.

## MbedTLS configuration file
Difference from original mbedTLS configuration.

Embedded system limitation (the next features are switched off):

- MBEDTLS_HAVE_TIME
- MBEDTLS_HAVE_TIME_DATE
- MBEDTLS_FS_IO
- MBEDTLS_NO_PLATFORM_ENTROPY
- MBEDTLS_NET_C
- MBEDTLS_PSA_CRYPTO_STORAGE_C
- MBEDTLS_PSA_ITS_FILE_C
- MBEDTLS_TIMING_C

Platform HW cryptographic features (the next features are switched on):

- MBEDTLS_ECP_ALT
- MBEDTLS_ENTROPY_HARDWARE_ALT
- MBEDTLS_AES_ALT

## HW limitation
- B91 platform elliptic curves unit support only 256-bits values as result all curves longer than 256 bits are processed by SW.
- B91 platform AES unit support only 128-bits keys as result all algorithms with key longer than 128 bits are processed by SW.

## Integration into project
Compile mbedtls library with providing mbedtls config file

    make lib CFLAGS="-I<path to this root folder from mbedtls makefile folder> -DMBEDTLS_CONFIG_FILE='<mbedtls_config.h>'"

Compile your sources together with this folder (provide mbedtls config file)

    CFLAGS   += -I<path to this root folder from project makefile folder> -DMBEDTLS_CONFIG_FILE='<mbedtls_config.h>'
    CXXLAGS  += -I<path to this root folder from project makefile folder> -DMBEDTLS_CONFIG_FILE='<mbedtls_config.h>'
    ASFLAGS  += -I<path to this root folder from project makefile folder> -DMBEDTLS_CONFIG_FILE='<mbedtls_config.h>'

Link your project against mbedTLS library

### Other dependencies
- MbedTLS uses memory allocation on heap (calloc, free) so check this functions on your platform
- MbedTLS test functionality uses printf - check it in your platform
- In case of using RTOS and it's planned to involve cryptography functionality from more than single thread implement
exclusive access for HW units in **internal/multithread.c** depending on your RTOS
