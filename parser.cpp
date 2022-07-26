enum class UNIT_TYPE { NDR = 1, IDR = 5, SEI, SPS, PPS, AUD, EOS };

namespace h264_frame {
	namespace keyframe { 
		namespace startcode { 
			const char four_bytes[] = { 00, 00, 00, 01 }; 
			const char three_bytes[] = { 00, 00, 01 };
		}
	}
	
	inline bool is_nal(char* data) {
		if ((memcmp(data, keyframe::startcode::four_bytes, sizeof(keyframe::startcode::four_bytes)) == 0)
			|| (memcmp(data, keyframe::startcode::three_bytes, sizeof(keyframe::startcode::three_bytes)) == 0)) {
			return true;
		}
		return false;
	}
	/*
	* @param char* data_cursor                      : video data buffer pointer
	* @param const int data_size                       : video data size
	* @param std::vector<std::vector<uint8_t>>& frames : out buffer
	*/
	void split_frames(char* data_cursor, const int data_size, std::vector<std::vector<char>>& frames) {
		long long read_idx = 0;
		auto push_frame = [](char* data, const int datasize, std::vector<std::vector<char>>& frames) {
			std::vector<char> frame_buff(datasize);
			memcpy(frame_buff.data(), data, datasize);
			frames.push_back(frame_buff);
		};

		while (read_idx < data_size) {
			int startcode_len = 0;
			if (read_idx <= data_size - 4) {
				if ((data_cursor[read_idx] == 0x00) && (data_cursor[read_idx + 1] == 0x00)) {
					if (data_cursor[read_idx + 2] == 0x01) { // keyframe::startcode::three_bytes
						startcode_len = sizeof(keyframe::startcode::three_bytes);
					}
					else if ((data_cursor[read_idx + 2] == 0x00) && (data_cursor[read_idx + 3] == 0x01)) { // keyframe::startcode::four_bytes
						startcode_len = sizeof(keyframe::startcode::four_bytes);
					}
				}
			}
			else {
				read_idx++;
				continue;
			}


			if (startcode_len > 0)
			{
				uint32_t  unit = *(data_cursor + read_idx + startcode_len);
				UNIT_TYPE unit_type = (UNIT_TYPE)((unit) & 0x1f); // 0001 1111
				std::vector<uint8_t> frame_buff;
				int sps_size = 3, pps_size = 3, idr_size = 3;
				switch (unit_type)
				{
				case UNIT_TYPE::SPS:
					while (!is_nal(data_cursor + read_idx + sps_size)) { // pps까지
						sps_size++;
						if (sps_size >= data_size) {
							break;
						}
					}
					push_frame(data_cursor + read_idx, sps_size, frames);
					read_idx += sps_size;
					continue;
				case UNIT_TYPE::PPS:
					while (!is_nal(data_cursor + read_idx + pps_size)) { // 다음 start code 까지
						pps_size++;
						if ((read_idx + pps_size) >= data_size) {
							break;
						}
					}
					push_frame(data_cursor + read_idx, pps_size, frames);
					read_idx += pps_size;
					continue;
					break;
				case UNIT_TYPE::NDR:
				case UNIT_TYPE::IDR:
					while (!is_nal(data_cursor + read_idx + idr_size)) { // 다음 start code 까지
						idr_size++;
						if ((read_idx + idr_size) >= data_size) {
							break;
						}
					}
					push_frame(data_cursor + read_idx, idr_size, frames);
					read_idx += idr_size;
					break;
				case UNIT_TYPE::EOS:
					push_frame(data_cursor + read_idx, data_size - read_idx, frames);
					read_idx += data_size - read_idx;
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
}

int read_file(const char* filepath, std::vector<char>& filebuffer) {
	ifstream vfile(filepath.c_str(), ios::binary);
	vfile.seekg(0, ios::end);
	int file_size = vfile.tellg();
	vfile.seekg(0, vfile.beg);
	
	filebuffer.resize(file_size);
	vfile.read(filebuffer.data(), file_size);
	return file_size;
}
