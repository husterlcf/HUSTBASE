#pragma once

class bitmanager
{
	public:
		bitmanager(int length, char *data);
		virtual ~bitmanager();
		bool atPos(int n);
		int firstBit(int n, bool val); //���ص�һ��ֵΪ0��λ��λ��
		bool anyZero();//λͼ���Ƿ���ֵΪ0��λ
		bool setBitmap(int pos, bool value); //����λͼ��ĳλ��ֵ
		//int setEffectiveLength(int length);
		void redirectBitmap(int length, char *data);

	protected:

	private:
		int bmLength;//λͼ��Ϣ��󳤶ȣ����ֽ�Ϊ��λ
		//char *bitmap;
		char *realData;
		//int effectiveLength;
};