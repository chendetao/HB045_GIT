copy ..\\Objects\app.hex .\\nRF52_DFU.hex
nrfutil dfu genpkg 8068_20180614111300_V1.0.8_alpha.zip --application nRF52_DFU.hex --application-version 0xFFFFFFFF --dev-revision 0xFFFF --dev-type 0xFFFF --sd-req 0xFFFE
del .\\nRF52_DFU.hex
pause