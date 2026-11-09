# Independent review: corrections and next steps

Date: 2026-11-09

## Material correction

This supersedes any implication in earlier notes that a BCCMD type-1 reply to SETREQ proves a write was converted into a read, or that type 3 is necessarily the expected write acknowledgement.

CSR's BCCMD Commands manual, bcore-sp-005Pe, shows SETREQ receiving GETRESP (type 1), including Config_UART on page 69 and a PS-key write example on page 71. [Manufacturer manual mirror](https://manualzz.com/doc/24213785/csr-bluecore-electronics-bccmd-commands-user-manual). [Scapy's CSR implementation](https://github.com/secdev/scapy/blob/master/scapy/contrib/bluetooth_vsc_csr.py) independently defines request 0, response 1 and write request 2.

Therefore previous ACK masks based on a required type-3 reply cannot establish failure. The response parser and recorded replies need reclassification against the documented protocol before concluding that the route rejects writes. This correction does not prove successful writes: immediate RAM read failures and unchanged baseline observations remain unresolved. Same-value writes also cannot establish a changed value.

The previously traced PSP-side copy path still shows no explicit type rewrite in the examined functions, but no rewrite is required to explain type 1. Request 0x70 remains insufficiently characterized; adjacent IDs must not be guessed.

## Additional research lead

CSR's [Understanding and Using Bootmodes, bcore-an-019Pd](https://www.protonbasic.co.uk/dev/Crownhill%20QuickBlue/bluetooth_project/bluecore/Understanding%20and%20Using%20Bootmodes%20%28bcore-an-019Pd%29.pdf), page 5, describes BCCMD_SECURITY_ACTIVE restrictions and bootmode-dependent PS-key mapping. These are possible factors to investigate, not findings that Sony enabled them or explanations of outer status 0x0050. No speculative security or bootmode writes are warranted.

## Independent software finding

The v0.9 host accepts fresh peers only when inquiry results provide EIR Device ID. It does not set an extended inquiry mode; the vendored stack initializes the mode to zero. Its configured standard discovery path consequently cannot select a fresh DS4 using that identity requirement.

A proposed fix is bounded candidate discovery followed by SDP Device ID verification (service 0x1200; vendor/source/product attributes from the same record), with provisional candidates separate from verified peers. Sony USB vendor 0x054C, vendor-source 2 and supported DS4 product IDs must be checked before enabling controls or persisting a bond. Names alone are insufficient. Actual controller capabilities must guide optional extended inquiry and SSP behavior; no legacy PIN should be invented.

This software issue is independent of the unresolved standard-HCI transport. Neither working HCI access nor DS4 control has been demonstrated.

## Immediate priority

Audit existing protocol parsing and captured replies first, then correct discovery with meaningful tests. These reviews did not execute hardware commands, flash firmware or establish a new HCI transition recipe.
