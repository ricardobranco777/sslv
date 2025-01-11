/*
 * OpenSSL / LibreSSL BoringSSL
 */
int
X509_verify_cert(void)
{
	return (1);
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
