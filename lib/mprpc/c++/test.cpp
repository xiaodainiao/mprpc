#include <iostream>
#include <string>
#include <vector>
//两个字符串，记录时间，开始时间;结束时间用分号割   小时：分钟


int main()
{
   vector<int> v = {1,2,5,10,21,21,40,5,6};
   vector<int> result;
   int target = 7;
   for (int i = 0; i < v.size(); i ++)
   {
    for (int j = i; j < v.size(); j ++)
    {
        if (v[i] + v[j] == target)
        {
            v.push_back(i);
            v.push_back(k);
            cout << "i = " << i << "j = " << j << endl;
        }
    }
   } 



    return ;

}