#ifndef LIGHTING_LIGHTMAP_H_
#define LIGHTING_LIGHTMAP_H_

#include <vector>
#include "../constants.h"
#include "../typedefs.h"

const int LIGHTMAP_DATA_LEN = CHUNK_VOL/2;

// Lichtkarte
class Lightmap {
public:
    std::vector<light_t> map = std::vector<light_t>(CHUNK_VOL, 0); // later it might be replaced with `fixed_vector` as its size should be a constant
    int highestPoint = 0;
    Lightmap() = default;
    ~Lightmap() = default;
    void set(const Lightmap& lightmap);
    void set(std::vector<light_t>&& map);
	void clear();

	inline unsigned short get(int x, int y, int z){
		return (map[y*CHUNK_D*CHUNK_W+z*CHUNK_W+x]);
	}

	inline unsigned char get(int x, int y, int z, int channel){
		return (map[y*CHUNK_D*CHUNK_W+z*CHUNK_W+x] >> (channel << 2)) & 0xF;
	}

	inline unsigned char getR(int x, int y, int z){
		return map[y*CHUNK_D*CHUNK_W+z*CHUNK_W+x] & 0xF;
	}

	inline unsigned char getG(int x, int y, int z){
		return (map[y*CHUNK_D*CHUNK_W+z*CHUNK_W+x] >> 4) & 0xF;
	}

	inline unsigned char getB(int x, int y, int z){
		return (map[y*CHUNK_D*CHUNK_W+z*CHUNK_W+x] >> 8) & 0xF;
	}

	inline unsigned char getS(int x, int y, int z){
		return (map[y*CHUNK_D*CHUNK_W+z*CHUNK_W+x] >> 12) & 0xF;
	}

	inline void setR(int x, int y, int z, int value){
		const int index = y*CHUNK_D*CHUNK_W+z*CHUNK_W+x;
		map[index] = (map[index] & 0xFFF0) | value;
	}

	inline void setG(int x, int y, int z, int value){
		const int index = y*CHUNK_D*CHUNK_W+z*CHUNK_W+x;
		map[index] = (map[index] & 0xFF0F) | (value << 4);
	}

	inline void setB(int x, int y, int z, int value){
		const int index = y*CHUNK_D*CHUNK_W+z*CHUNK_W+x;
		map[index] = (map[index] & 0xF0FF) | (value << 8);
	}

	inline void setS(int x, int y, int z, int value){
		const int index = y*CHUNK_D*CHUNK_W+z*CHUNK_W+x;
		map[index] = (map[index] & 0x0FFF) | (value << 12);
	}

	inline void set(int x, int y, int z, int channel, int value){
		const int index = y*CHUNK_D*CHUNK_W+z*CHUNK_W+x;
		map[index] = (map[index] & (0xFFFF & (~(0xF << (channel*4))))) | (value << (channel << 2));
	}

    inline const std::vector<light_t>& getLights() const {
        return map;
    }

    inline std::vector<light_t>& getLights() {
        return map;
    }

	static inline light_t combine(int r, int g, int b, int s) {
		return r | (g << 4) | (b << 8) | (s << 12);
	}

	static inline light_t extract(light_t light, ubyte channel) {
		return (light >> (channel << 2)) & 0xF;
	}

	std::vector<ubyte> encode() const;
	static std::vector<light_t> decode(const std::vector<ubyte>& buffer);
};

#endif /* LIGHTING_LIGHTMAP_H_ */
