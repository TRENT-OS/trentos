# CertParser API

## Overview

The CertParser API provides functionality for parsing and validating
X.509 certificates.

### Concepts

The following describes two concepts relevant for this API, which are
related to the basic data structures handled by the parser.

#### Certs

The CertParser handles certs, which need to be initialized from a buffer
(can be PEM or DER encoded). Some fields of a certificate can be
extracted from a cert. Furthermore, certs can be added to chains, as can
be seen below.

#### Chains

In order to organize a set of certificates -- which may have a
hierarchical dependency based on their respective signatures -- certs
can be organized into chains. A chain can have a single cert or multiple
certs attached to it. Chains simply contain references to certs, so it
has to be taken care that a cert is not freed, while it is still in use
in a chain.

To ensure that chains are consistent, when adding a new cert to a chain
it is checked that the new cert is signed by the last cert of the chain
(in case it has non-zero length).

The main operation of the CertParser, which is the verification of
certificates, operates on the basis of chains, i.e., an input chain is
validated against another chain. For this, the CertParser requires at
least one chain added to it as "trusted chain", against which the
verification of input chains can be executed.

### Architecture

The CertParser offers a simple interface and internally uses the
[Crypto API](crypto_api.md) to perform signature verification and hash
calculation.

In addition, the CertParser uses the X.509 parser code provided by mbedTLS to
handle the complex X.509 data structures.

!["CertParser API - Architecture"](img/cert-parser_api-architecture.png)

## Usage

This is how the API can be instantiated in the system.

### Declaration of API Library in CMake

To link the CertParser implementation to a project, the following build
target can be used:

- **`os_cert`**: Builds the complete CertParser library and all
    third-party dependencies

Since the CertParser uses the Crypto API to perform cryptographic
operations, the respective build target needs to be included as well.

## Example

### Instantiation of API in CMake

Here we show how a component can include the CertParser (and the Crypto
API library) into the build process:

```c
DeclareCAmkESComponent(
    Client
    SOURCES
        ...
    C_FLAGS
        ...
    LIBS
        ...
        os_cert
        os_crypto
)
```

### Using the API in C

In the following example, the CertParser is set up with a trusted chain of two
certificates (a "root certificate" and an "intermediate certificate").

Then a second chain is set up with a "server certificate"; this chain is then
verified against the trusted chain.

Finally, all chains are freed; please note that all associated certs are
freed automatically by setting the appropriate parameter to **`true`**.

```c
// For CertParser
#import "OS_CertParser.h"

// For Entropy component
#include <camkes.h>

// Configuration of Crypto API in "LIBRARY_ONLY" mode
static OS_Crypto_Config_t cfgCrypto = {
    .mode    = OS_Crypto_MODE_LIBRARY,
    ...
};

static OS_CertParserConfig_t parserCfg;

static const uint8_t rootCert[] =                                           \
    "-----BEGIN CERTIFICATE-----\r\n"                                       \
    "MIIDkDCCAnegAwIBAgIUXEphp/RzjJH6jYvDDsBQdrGYRSowDQYJKoZIhvcNAQEL\r\n"  \
    ...                                                                     \
    "1oHJU/rq1kJljRXXZRO695Uk46IhPNDhAu1L0ml6ansEz9Fl28Yal0Z2+zL0rUAz\r\n"  \
    "gqZivg==\r\n"                                                          \
    "-----END CERTIFICATE-----\r\n";

static const uint8_t imedCert[] =                                           \
    "-----BEGIN CERTIFICATE-----\r\n"                                       \
    "MIIDNzCCAh4CFHbRcbAo6lHOaFjcHj8WsC4A0W+wMA0GCSqGSIb3DQEBCwUAMFcx\r\n"  \
    ...                                                                     \
    "p7KAL9QH/TF7cq73sRSGnRiHlIO+j5jUG25Hcb8IQgSzZ64tF56iGyNhLeieLHTt\r\n"  \
    "Ht77hK6Ffknm4lY=\r\n"                                                  \
    "-----END CERTIFICATE-----\r\n";

static const uint8_t serverCert[] =
    "-----BEGIN CERTIFICATE-----\r\n" \
    "MIIDJzCCAg8CAWUwDQYJKoZIhvcNAQELBQAwWDELMAkGA1UEBhMCQVUxEzARBgNV\r\n"  \
    ...                                                                     \
   "r4iuTnKYkHJAV7j5gW8uIlVaasPQbrf0fH511A8oFeeNt6ik1KIUPD9sF+5qG3Yv\r\n"   \
    "UHBt9lKNPJ0Zz1AaYGW0vrE6gHB1Ql+5bNyWnFON5pnvnPBUXA96KPNueA==\r\n"      \
    "-----END CERTIFICATE-----\r\n";
...

int run()
{
    OS_CertParsert_Handle_t parser;
    OS_CertParserChain_Handle_t chain;
    OS_CertParserCert_Handle_t cert;
    OS_CertParser_VerifyFlags_t flags;

    // Set up Crypto API and parser instance
    OS_Crypto_init(&parserCfg.hCrypto, &cfgCrypto);
    OS_CertParser_init(&parser, &parserCfg);

    // Construct chain of root and intermediate cert
    OS_CertParserChain_init(&chain, parser);
    OS_CertParserCert_init(&cert,
                            parser,
                            OS_CertParserCert_Encoding_PEM,
                            rootCert,
                            sizeof(rootCert));
    OS_CertParserChain_addCert(chain, cert);
    OS_CertParserCert_init(&cert,
                            parser,
                            OS_CertParserCert_Encoding_PEM,
                            imedCert,
                            sizeof(imedCert));
    OS_CertParserChain_addCert(chain, cert);

    // Add chain to parser; this chain now has index=0 as it is the first chain
    OS_CertParser_addTrustedChain(parser, chain);

    // Create a chain for the certificate we want to verify
    OS_CertParserChain_init(&chain, parser);
    OS_CertParserCert_init(&cert,
                           parser,
                           OS_CertParserCert_Encoding_PEM,
                           serverCert,
                           sizeof(serverCert));
    OS_CertParserChain_addCert(chain, cert);

    // Verify new chain against CA chain with index=0, i.e.,the chain of root
    // and intermediate cert
    if (OS_CertParser_verifyChain(parser, 0, chain, &flags) != OS_SUCCESS) {
        printf("Verification failed! Error flags = %x\n", flags);
    }

    // Free up chain again and also the server cert referenced by it
    OS_CertParserChain_free(chain, true);

    // Free parser and crypto; also free up all associated chains (the CA chain
    // we added in the first step)
    OS_CertParser_free(parser, true);
    OS_Crypto_free(parserCfg.hCrypto);
}
```
