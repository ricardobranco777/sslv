#!/bin/sh

PORT="7777"
CERT_FILE="cert.pem"
KEY_FILE="key.pem"
LIBSSL_PRELOAD="./libsslk.so"

set -eu

cleanup() {
	set +e
	if [ -n "$SERVER_PID" ] ; then
		kill "$SERVER_PID"
		wait "$SERVER_PID"
	fi
	rm -f "$KEY_FILE" "$CERT_FILE"
}
trap cleanup EXIT

# Generate a self-signed certificate and private key
openssl req -x509 -newkey rsa:4096 -keyout "$KEY_FILE" -out "$CERT_FILE" -days 365 -nodes -subj "/CN=localhost" >/dev/null 2>&1

# Start the OpenSSL test server
openssl s_server -accept "$PORT" -key "$KEY_FILE" -cert "$CERT_FILE" -www >/dev/null 2>&1 &
SERVER_PID=$!

sleep 1

# Test function
run_test() {
	test="$1"
	command="$2"

	echo "Test $test without LD_PRELOAD"
	if ! eval "$command" ; then
		echo "PASS"
	else
		echo "FAIL"
	fi
	echo

	echo "Test $test with LD_PRELOAD"
	if eval LD_PRELOAD="$LIBSSL_PRELOAD" "$command" ; then
		echo "PASS"
	else
		echo "FAIL"
	fi
	echo
}

# Test cases
run_test "OpenSSL client" \
	"openssl s_client -no-interactive -quiet -verify_quiet -verify_return_error -servername localhost -connect localhost:$PORT </dev/null 2>&1"

run_test "GnuTLS client" \
	"gnutls-cli --logfile /dev/null --port $PORT localhost </dev/null"
