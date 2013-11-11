all: oci.dll install

oci.dll: oci_stub.c oci_nonstub.c
	cl oci_stub.c oci_nonstub.c /link /DLL /out:oci.dll

install:
	copy oci.dll c:\build\tora-build\release\oci.dll

clean:
	del *~ *.exp *.obj *.dll *.lib
