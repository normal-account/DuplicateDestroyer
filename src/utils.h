#include <utility>


class Chrono
{
public :

   Chrono(std::string name, bool showWhenDestroyed=true)
           : _name(std::move(name)), _duree(0),_dureeSec(0), _pause(false), _showWhenDestroyed(showWhenDestroyed)
   {
       gettimeofday(&depart, &tz);
   }

   Chrono()
           : _duree(0),_dureeSec(0), _pause(false)
   {
       gettimeofday(&depart, &tz);
   }

   ~Chrono() {
       if(!_name.empty() && _showWhenDestroyed)
               print();
   }

   void setDuree(long sec, long microSec=0)
   {
       _duree= sec * 1000000L + microSec;
       _dureeSec=sec;
   }

   void tic()
   {
       _pause=false;
       _duree=0;
       _dureeSec=0;
       gettimeofday(&depart, &tz);
   }

   long pause(bool val)
   {
       if(val)
       {
           if(!_pause)
           {
               gettimeofday(&fin, &tz);
               _duree += (fin.tv_sec-depart.tv_sec) * 1000000L + (fin.tv_usec-depart.tv_usec);
               _dureeSec += fin.tv_sec-depart.tv_sec ;
               _pause=true;
           }
       }else
       {
           if(_pause)
           {
               gettimeofday(&depart, &tz);
               _pause=false;
           }
       }
       return _duree;
   }

   long pauseSec(bool val)
   {
       if(val)
       {
           if(!_pause)
           {
               gettimeofday(&fin, &tz);
               _duree += (fin.tv_sec-depart.tv_sec) * 1000000L + (fin.tv_usec-depart.tv_usec);
               _dureeSec += fin.tv_sec-depart.tv_sec ;
               _pause=true;
           }
       }else
       {
           if(_pause)
           {
               gettimeofday(&depart, &tz);
               _pause=false;
           }
       }
       return _dureeSec;
   }

   long tac()
   {
       if(!_pause)
       {
           gettimeofday(&fin, &tz);
           return (fin.tv_sec-depart.tv_sec) * 1000000L + (fin.tv_usec-depart.tv_usec) + _duree;
       }else
       {
           return _duree;
       }
   }

   long tacSec()
   {
       if(!_pause)
       {
           gettimeofday(&fin, &tz);
           return (fin.tv_sec-depart.tv_sec) + _dureeSec;
       }else
       {
           return _dureeSec;
       }
   }

   void print()
   {
       auto val = (double)tac();
       if(!_name.empty())
           std::cout << _name << ": ";
       if(val < 1000.0)
           std::cout << val << " µs" << std::endl;
       else if(val < 1000000.0)
           std::cout << val/1000.0 << " ms" << std::endl;
       else
           std::cout << val/1000000.0 << " sec" << std::endl;
   }

   void showWhenDestroyed(bool val) {
       _showWhenDestroyed = val;
   }

private :


   std::string _name;
   struct timeval depart{}, fin{};
   struct timezone tz{};
   long _duree;
   long _dureeSec;

   bool _pause;
   bool _showWhenDestroyed{};
};