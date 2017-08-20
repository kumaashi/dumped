#pragma once

unsigned long random()
{
	static unsigned long a = 9012345;
	static unsigned long b = 2345678;
	static unsigned long c = 9012345;
	a += b;
	b += c;
	c += a;
	return(a >> 16);
}

float frandom()
{
	return float(random()) / float(0xFFFF);
}

float frandom2()
{
	return frandom() * 2.0 - 1.0;
}

struct File {
	std::vector<unsigned char> buf;
	size_t Size() {
		return buf.size();
	}

	File(const char *name, const char *mode = "rb") {
		FILE *fp = fopen(name, mode);
		if (fp) {
			fseek(fp, 0, SEEK_END);
			buf.resize(ftell(fp));
			fseek(fp, 0, SEEK_SET);
			fread(&buf[0], 1, buf.size(), fp);
			fclose(fp);
		} else {
			printf("Cant Open File -> %s\n", name);
		}
	}

	void *Buf() {
		return (void *)(&buf[0]);
	}
};

