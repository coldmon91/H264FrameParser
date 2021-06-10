
namespace h264_frames {
	namespace keyframe {
		namespace four_bytes {
			const uint8_t startcode[] = { 00, 00, 00, 01 };
		}
		namespace three_bytes {
			const uint8_t startcode[] = { 00, 00, 01 };
		}
	}
	bool IsNalUnit(uint8_t* data) {
		if ((memcmp(data, keyframe::four_bytes::startcode, sizeof(keyframe::four_bytes::startcode)) == 0)
			|| (memcmp(data, keyframe::three_bytes::startcode, sizeof(keyframe::three_bytes::startcode)) == 0)) {
			return true;
		}
		return false;
	}

    /*
    * @param uint8_t* data_cursor                      : video data buffer pointer
    * @param const int data_size                       : video data size
    * @param std::vector<std::vector<uint8_t>>& frames : out buffer
    */
	void split_frames(uint8_t* data_cursor, const int data_size, std::vector<std::vector<uint8_t>>& frames) {
		long long read_idx = 0;
		auto PushFrame = [&](uint8_t*& data, const int datasize, std::vector<std::vector<uint8_t>>& frames) {
			std::vector<uint8_t> frame_buff(datasize);
			memcpy(frame_buff.data(), data, datasize);
			frames.push_back(frame_buff);
			data += datasize;
			read_idx += datasize;
		};
		while (read_idx < data_size)
		{
			int startcode_len = 0;
			if ((memcmp(data_cursor, keyframe::four_bytes::startcode, sizeof(keyframe::four_bytes::startcode)) == 0)) {
				startcode_len = sizeof(keyframe::four_bytes::startcode);
			}
			else if ((memcmp(data_cursor, keyframe::three_bytes::startcode, sizeof(keyframe::three_bytes::startcode)) == 0)) {
				startcode_len = sizeof(keyframe::three_bytes::startcode);
			}
			else {
				read_idx++;
				continue;
			}
			if (startcode_len > 0)
			{
				uint32_t  unit = *(data_cursor + startcode_len);
				UNIT_TYPE unit_type = (UNIT_TYPE)((unit) & 0x1f); // 0001 1111
				std::vector<uint8_t> frame_buff;
				int sps_size = 3, pps_size = 3, idr_size = 3;
				switch (unit_type)
				{
				case UNIT_TYPE::SPS:
					while (!IsNalUnit(data_cursor + sps_size)) { // pps까지
						sps_size++;
						if (sps_size >= data_size) {
							break;
						}
					}
					PushFrame(data_cursor, sps_size, frames);
					continue;
				case UNIT_TYPE::PPS: 
					while (!IsNalUnit(data_cursor + pps_size)) { // 다음 start code 까지
						pps_size++;
						if ((read_idx + pps_size) >= data_size) {
							break;
						}
					}
					PushFrame(data_cursor, pps_size, frames);
					continue;
					break;
				case UNIT_TYPE::NDR:
				case UNIT_TYPE::IDR:
					while (!IsNalUnit(data_cursor + idr_size)) { // 다음 start code 까지
						idr_size++;
						if ((read_idx + idr_size) >= data_size) {
							break;
						}
					}
					PushFrame(data_cursor, idr_size, frames);
					break;
				case UNIT_TYPE::EOS:
					PushFrame(data_cursor, data_size - read_idx, frames);
					break;
				case UNIT_TYPE::SEI:
				case UNIT_TYPE::AUD:
					break;
				default:
					break;
				}
			}
		}
	}

    /*
    * @param const char* filepath             : h264file full path
    * @param std::vector<uint8_t>& filebuffer : out buffer
    */
	int read_file(const char* filepath, std::vector<uint8_t>& filebuffer) {
		FILE* videofile = fopen(filepath, "rb");
		const int buffsize = 65535;
        filebuffer.clear();
        filebuffer.resize(buffsize);
		int size = 0, totalread = 0;
		while ((size = fread(filebuffer.data() + totalread, 1, buffsize, videofile)) > 0) {
			totalread += size;
            if ((filebuffer.size() - totalread) < buffsize) {
			    filebuffer.resize(filebuffer.size() + buffsize);
            }
		}
        filebuffer.resize(totalread);
        return filebuffer.size();
	}

}