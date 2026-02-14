/*
 * OpenSSL / BoringSSL
 */
int
X509_verify_cert(void)
{
	return (1);
}

/* LibreSSL */
int
_libre_X509_verify_cert(void)
{
	return (1);
}

long
SSL_get_verify_result(void)
{
	return (0);
}

/*
 * GnuTLS
 */
int
gnutls_certificate_verify_peers(void)
{
	return (0);
}

int
gnutls_session_get_verify_cert_status(void)
{
	return (0);
}

/*
 * NSS
 */
int
CERT_VerifyCert(void)
{
	return (0);
}

int
CERT_VerifyCertificate(void)
{
	return (0);
}

#include <stdlib.h>
#include <string.h>
#include <gpgme.h>

gpgme_error_t
gpgme_op_verify(
	__attribute__ ((unused)) gpgme_ctx_t ctx,
	__attribute__ ((unused)) gpgme_data_t sig,
	__attribute__ ((unused)) gpgme_data_t signed_text,
	__attribute__ ((unused)) gpgme_data_t plaintext)
{
	return (0);
}

gpgme_error_t
gpgme_op_verify_ext(
	__attribute__ ((unused)) gpgme_ctx_t ctx,
	__attribute__ ((unused)) gpgme_verify_flags_t flags,
	__attribute__ ((unused)) gpgme_data_t sig,
	__attribute__ ((unused)) gpgme_data_t signed_text,
	__attribute__ ((unused)) gpgme_data_t plain)
{
	return (0);
}

static gpgme_verify_result_t fake_result;

gpgme_verify_result_t
gpgme_op_verify_result(
	__attribute__ ((unused)) gpgme_ctx_t ctx)
{
	if (fake_result)
		return fake_result;

	fake_result = calloc(1, sizeof(*fake_result));
	fake_result->signatures = calloc(1, sizeof(*fake_result->signatures));

	gpgme_signature_t sig = fake_result->signatures;
	sig->fpr = strdup("0123456789ABCDEF0123456789ABCDEF01234567");
	sig->validity = GPGME_VALIDITY_FULL;

	return fake_result;
}
