#ifndef ALLOCATOR_H___
#define ALLOCATOR_H___

#include "config.h"
#include "funcs.h"
#include <map>
#include <assert.h>

namespace Wrap{
	//�ڴ���������
	class Allocator{
	public:
		struct LeakPosition{
			char file[250];
			long line;
			size_t size;
		};

	private:
		Allocator(){}
		virtual ~Allocator(){
			assert(mPointerMap.size() == 0);//�쳣�Ļ����ڴ�й©�ˡ���

			std::map<void*, LeakPosition>::iterator it = mPointerMap.begin();
			for (it; it != mPointerMap.end(); it++){
				LOGE("Mem Leak for %s %ld", it->second.file, it->second.line);
			}
		}

	public:

		/*
		@size  ����������ڴ��С
		*/
		virtual void* alloc(size_t size, const char* file, long line){
			void* pointer = calloc(1, size);
			//LOGD("alloc mem pointer = %p,size = %d", pointer, size);

			LeakPosition lp;
			STR_CPY(lp.file, file);
			lp.line = line;
			lp.size = size;
			mPointerMap.insert(std::make_pair(pointer, lp));
			return pointer;
		}
		virtual void dealloc(void* pointer){
			if (pointer != NULL){
				//LOGD("dealloc mem pointer = %p", pointer);
				mPointerMap.erase(pointer);
				free(pointer);
			}
		}

		static Allocator* GetInstance(){
			if (sIns == NULL){
				sIns = new Allocator();
			}
			return sIns;
		}
		static Allocator* ReleaseIns(){
			delete sIns;
			sIns = NULL;
		}

	private:
		static Allocator* sIns;
		std::map<void*,LeakPosition> mPointerMap;
	};
}

#define wrap_calloc(M_o) \
	Wrap::Allocator::GetInstance()->alloc(M_o,__FILE__,__LINE__)

#define wrap_free(M_o) \
	Wrap::Allocator::GetInstance()->dealloc((void*)M_o);\
	M_o = NULL

//ͨ�ö���� new_��delete_�����Բ��ü̳�Object,һ���հ�ֻ������һ�μ���
#define wrap_new_begin void* M_temp = NULL
//��ʹ��new֮ǰ��������new_begin;
#define wrap_new(M_T,...) \
	(M_temp = Wrap::Allocator::GetInstance()->alloc(sizeof(M_T), __FILE__, __LINE__)) == NULL ? NULL : new (M_temp)M_T(__VA_ARGS__)

//�ͷŶ���,�����namespace����ô����M_T��ʱ�����ȥ��
//���о�����ð���������д��������,��������������������޷�ִ��
#define wrap_delete(M_T,M_o) \
	if (M_o != NULL){\
	M_o->~M_T();\
	Wrap::Allocator::GetInstance()->dealloc(M_o);\
	M_o = NULL;\
}

#endif