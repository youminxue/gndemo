#include "base/at_exit.h"
#include "base/bind.h"
#include "base/callback.h"
#include <iostream>
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/threading/simple_thread.h"
using namespace base;
void speak1(void *param){
    std::cout<<"exit1"<<std::endl;
}
void speak2(void *param){
    std::cout<<"exit2"<<std::endl;
}
void speak3(std::string name, void* param){
    std::cout<<"exit3"<<name<<std::endl;
}
int sum(int x,int y,int z){
    return x+y+z;
}
class People{
    public:
        People(std::string tmp){name = tmp;};
        std::string name;
        void say(std::string info){
            std::cout<<name<<":"<<info<<std::endl;
        }
        void hi(std::string info){
            std::cout<<name<<info<<std::endl;
        }
};

class TaskDelegate:public DelegateSimpleThread::Delegate{
    public:
        void Run() override{
            std::cout<<"youzhenyu"<<std::endl;
        };
};

int main(){
    AtExitManager exitmanager;
    AtExitManager::RegisterCallback(speak1, nullptr);
    AtExitManager::RegisterCallback(speak2, nullptr);
    AtExitManager::RegisterTask(Bind(speak3, "youzhenyu", nullptr));
    AtExitManager exitmanager2;
    // exitmanager2.RegisterTask(Bind(speak3, "youzhenyu", nullptr));
    // //BindRepeating可以重复被定义
    // RepeatingCallback<int(int)> c0 = BindRepeating(&sum, 12, 12);
    // std::cout<<c0.Run(12)<<std::endl;
    // std::cout<<c0.Run(13)<<std::endl;
    // RepeatingCallback<int(void)> c1 = BindRepeating(c0, 10);
    // std::cout<<c1.Run()<<std::endl;
    // //BindOnce只可以调用一次,调用Run必须转换右值
    // //onecallback变量必须使用move转换成右值，临时变量则不需要
    // OnceCallback<int(int)> c2= BindOnce(&sum, 12, 12);
    // std::cout<<std::move(c2).Run(12)<<std::endl;
    // std::cout<<c2.is_null()<<std::endl;

    // RepeatingCallback<int(int)> c4 = BindRepeating(&sum, 12, 12);
    // std::cout<<BindOnce(c4).Run(12)<<std::endl;
    MessageLoopForIO message_loop;
    MessageLoop::current()->task_runner()->PostTask(FROM_HERE, BindOnce(&speak1, nullptr));
    MessageLoop::current()->task_runner()->PostDelayedTask(FROM_HERE, BindOnce(&speak3,"youzhenyu", nullptr), TimeDelta::FromSeconds(2));
    
    People* cp = new People("youzhenyu");
    // auto cp = MakeRefCounted<People>();
    Callback<void()> no_ref_const_cb = Bind(&People::hi, Unretained(cp), " welcome");
    no_ref_const_cb.Run();
    DelegateSimpleThreadPool poll("ytest", 12);
    poll.Start();

    poll.AddWork(new TaskDelegate(), 10000);
    poll.JoinAll();
    std::cout<<"finish"<<std::endl;
    RunLoop().Run();
    // no_ref_const_cb.Run();
    // base::Callback<void()> call = base::Bind(&People::hi, &people);
    // call.Run();
    return 0;
}