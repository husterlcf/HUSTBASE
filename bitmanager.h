#pragma once

class bitmanager
{
	public:
		bitmanager(int length, char *data);
		virtual ~bitmanager();
		bool atPos(int n);
		int firstBit(int n, bool val); //返回第一个值为0的位的位置
		bool anyZero();//位图中是否有值为0的位
		bool setBitmap(int pos, bool value); //设置位图中某位的值
		//int setEffectiveLength(int length);
		void redirectBitmap(int length, char *data);

	protected:

	private:
		int bmLength;//位图信息最大长度，以字节为单位
		//char *bitmap;
		char *realData;
		//int effectiveLength;
};