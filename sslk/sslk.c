/*
 * OpenSSL / LibreSSL BoringSSL
 */
int
X509_verify_cert(void *ctx __attribute__((unused)))
{
	return (1);
}

/*
 * GnuTLS
 */
int
gnutls_certificate_verify_peers(
	void *session __attribute__((unused)),
	void *data __attribute__((unused)),
	unsigned int elements __attribute__((unused)),
	unsigned int *status __attribute__((unused)))
{
	return (0);
}
