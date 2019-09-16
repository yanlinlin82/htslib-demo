#include <cstdio>
#include <string>
#include "htslib/hts.h"
#include "htslib/sam.h"

int main(int argc, char** const argv)
{
	if (argc < 3) {
		printf("Usage: %s <in.bam> <out.bam>\n", argv[0]);
		return 1;
	}

	samFile* fp_in = sam_open(argv[1], "rb");
	if (!fp_in) {
		printf("Error: Failed to open '%s'!\n", argv[1]);
		return 1;
	}

	samFile* fp_out = sam_open(argv[2], "wb");
	if (!fp_out) {
		printf("Error: Failed to create '%s'!\n", argv[2]);
		sam_close(fp_in);
		return 1;
	}
	
	hts_set_threads(fp_in, 3); // create 3 threads for reading BAM
	hts_set_threads(fp_out, 4); // create 4 threads for writing BAM

	bam_hdr_t* hdr = sam_hdr_read(fp_in);
	if (!hdr) {
		printf("Error: Failed to read BAM head!\n");
		sam_close(fp_out);
		sam_close(fp_in);
		return 1;
	}
	int r = sam_hdr_write(fp_out, hdr);
	if (r < 0) {
		printf("Error: Failed to write BAM head!\n");
		bam_hdr_destroy(hdr);
		sam_close(fp_out);
		sam_close(fp_in);
		return 1;
	}

	bam1_t* b = bam_init1();
	if (!b) {
		printf("Error: Failed to init BAM block!\n");
		bam_hdr_destroy(hdr);
		sam_close(fp_out);
		sam_close(fp_in);
		return 1;
	}
	for (;;) {
		r = sam_read1(fp_in, hdr, b);
		if (r < 0) {
			if (r < -1) {
				printf("Error: truncated file or corrupt BAM index! r = %d\n", r);
			}
			break;
		}

		const char* s = bam_get_qname(b);
		size_t len = b->core.l_qname - 1 - b->core.l_extranul;
		std::string read_name(s, s + len); // here we got the read name
		std::string::size_type pos = read_name.find('#'); // suppose read name format as '@XXX:XXX:XXX#ACTGACTG', barcode is after '#'
		if (pos != std::string::npos) { // we found '#'
			std::string barcode = read_name.substr(pos);

			r = sam_write1(fp_out, hdr, b);
			if (r <= 0) {
				printf("Error: Failed to write! r = %d\n", r);
				break;
			}
		}
	}

	bam_hdr_destroy(hdr);
	sam_close(fp_out);
	sam_close(fp_in);
	return 0;
}
