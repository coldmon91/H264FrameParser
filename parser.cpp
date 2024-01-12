
enum class UNIT_TYPE { NDR = 1, IDR = 5, SEI, SPS, PPS, AUD, EOS };

namespace h264_frame {
	namespace keyframe {
		namespace startcode {
			const char four_bytes[] = { 00, 00, 00, 01 };
			const char three_bytes[] = { 00, 00, 01 };
		}
	}

	inline int is_startcode(uint8_t* cursor, int size) {
		if (size < 4) { return 0; }
		if ((cursor[0] ^ 0x00 | cursor[1] ^ 0x00 | cursor[2] ^ 0x00 | cursor[3] ^ 0x01) == 0) {
			return 4;
		}
		else if ((cursor[0] ^ 0x00 | cursor[1] ^ 0x00 | cursor[2] ^ 0x01) == 0) {
			return 3;
		}
		return 0;
	}

	/*
	 * Table 7-1 – NAL unit type codes, syntax element categories, and NAL unit type classes in
	 * T-REC-H.264-201704
	 */
	enum {
		H264_NAL_UNSPECIFIED = 0,
		H264_NAL_SLICE = 1,
		H264_NAL_DPA = 2,
		H264_NAL_DPB = 3,
		H264_NAL_DPC = 4,
		H264_NAL_IDR_SLICE = 5,
		H264_NAL_SEI = 6,
		H264_NAL_SPS = 7,
		H264_NAL_PPS = 8,
		H264_NAL_AUD = 9,
		H264_NAL_END_SEQUENCE = 10,
		H264_NAL_END_STREAM = 11,
		H264_NAL_FILLER_DATA = 12,
		H264_NAL_SPS_EXT = 13,
		H264_NAL_PREFIX = 14,
		H264_NAL_SUB_SPS = 15,
		H264_NAL_DPS = 16,
		H264_NAL_RESERVED17 = 17,
		H264_NAL_RESERVED18 = 18,
		H264_NAL_AUXILIARY_SLICE = 19,
		H264_NAL_EXTEN_SLICE = 20,
		H264_NAL_DEPTH_EXTEN_SLICE = 21,
		H264_NAL_RESERVED22 = 22,
		H264_NAL_RESERVED23 = 23,
		H264_NAL_UNSPECIFIED24 = 24,
		H264_NAL_UNSPECIFIED25 = 25,
		H264_NAL_UNSPECIFIED26 = 26,
		H264_NAL_UNSPECIFIED27 = 27,
		H264_NAL_UNSPECIFIED28 = 28,
		H264_NAL_UNSPECIFIED29 = 29,
		H264_NAL_UNSPECIFIED30 = 30,
		H264_NAL_UNSPECIFIED31 = 31,
	};
	/*
	* @param char* data_cursor                      : video data buffer pointer
	* @param const int data_size                       : video data size
	* @param std::vector<std::vector<uint8_t>>& frames : out buffer
	*/
	void split_frames(char* data_cursor, const int data_size, std::vector<std::vector<char>>& frames) {
		printf("split_frames...\n");
		std::vector<char> frame;
		int remain_size = data_size;
		int spspps = 1;
		for (int i = 0; i < data_size; i++) {
			int code = is_startcode((uint8_t*)data_cursor + i, remain_size);
			if (code > 0) {
				uint8_t type = (uint8_t)(data_cursor[i + code]) & 0x1F;
				switch (type)
				{
				case 1: //idr
				case 5: //non-idr, P
					if (!frame.empty()) {
						frames.push_back(frame);
						frame.clear();
					}
					break;
				case 6: //sei
				case 7: //sps
				case 8: //pps
				default:
					if (!frame.empty()) {
						frames.push_back(frame);
						frame.clear();
					}
					break;
				}
				for (int j = 0; j < code; i++, j++) {
					frame.push_back(data_cursor[i]);
					remain_size--;
				}
			}
			frame.push_back(data_cursor[i]);
			remain_size--;
		}
		if (!frame.empty()) {
			frames.push_back(frame);
			frame.clear();
		}

	}
}
