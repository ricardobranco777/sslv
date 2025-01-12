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

/*
 * NSS
 */
int
CERT_VerifyCert(void)
{
	return (0);
}
