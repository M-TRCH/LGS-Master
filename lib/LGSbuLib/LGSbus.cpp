
#include "LGSbus.h"

LGSbus::LGSbus()
{

}

LGSbus::~LGSbus()
{
  
}

void LGSbus::begin(int id, RS485Class* serial, int baud, int timeout)
{ 
  serial->begin(abs(baud));
  serial->setTimeout(abs(timeout));
  SR = serial;
  ID = constrain(id, 0, 255);
}

void LGSbus::setReturnDelayTime(int t)
{
  retDelayTime = abs(t);
}

int LGSbus::send(int id, int type, int len, int addr, int timeout, int cnt_max)
{
  // 1. prepare
  addr = constrain(addr, 0, DATA_BUF_SIZE-1); 
  timeout = abs(timeout);
  cnt_max = abs(cnt_max);
  int cnt = 0;
    
  while(cnt < cnt_max)  // times exceeded
  { 
    cnt++;
    int sum;
    
    // 2. send
    SR->beginTransmission();
    SR->write('#');
    SR->println(id);
    SR->println(type);
    SR->println(len);
    SR->println(addr);
    sum = id + type + len + addr;
    for (int i=0; i<len; i++)
    {
      SR->println(DATA_BUF[addr][i]);
      sum += DATA_BUF[addr][i];
    }
    SR->println(sum);  
    SR->endTransmission();
    
  
    // 3. return
    unsigned long startTime = millis();
    while(millis()-startTime < timeout) // timeout
    { 
      if (SR->find('&'))
      {
        int sum_cal, data_ret[DATA_SUBBUF_SIZE];

        // get packet 
        int id_ret      = SR->parseInt();
        int type_ret    = SR->parseInt();
        int len_ret     = SR->parseInt();
        int addr_ret    = SR->parseInt();
        len_ret = constrain(len, 1, DATA_SUBBUF_SIZE);
        sum_cal = id_ret + type_ret + len_ret + addr_ret;
        for (int i=0; i<len_ret; i++)
        {
          data_ret[i] = SR->parseInt();
          sum_cal += data_ret[i]; 
        }
        int sum_ret = SR->parseInt();

        // put data into the buffer
        if (id==id_ret && type_ret==RETURN_TYPE && addr==addr_ret && sum_cal==sum_ret)
        {
          addr_ret = constrain(addr_ret, 0, DATA_BUF_SIZE-1);
          for (int i=0; i<len; i++)
          {
            DATA_BUF[addr_ret][i] = data_ret[i];
          }
          return 1;
        }
        else  // packet incorrect 
        {
          break;
        }
      }
    }
  }
  return 0;
}

int LGSbus::readData(int addr, int index)
{
  addr = constrain(addr, 0, DATA_BUF_SIZE-1); 
  index = constrain(index, 0, DATA_SUBBUF_SIZE-1);
  return DATA_BUF[addr][index];
}

void LGSbus::writeData(int addr, int index, int data)
{
  addr = constrain(addr, 0, DATA_BUF_SIZE-1); 
  index = constrain(index, 0, DATA_SUBBUF_SIZE-1);
  DATA_BUF[addr][index] = data;
}

int LGSbus::write(int id, int len, int addr, int timeout, int cnt_max)
{
  return send(id, WRITE_TYPE, len, addr, timeout, cnt_max);
}

int LGSbus::read(int id, int addr, int timeout, int cnt_max)
{
  return send(id, READ_TYPE, 1, addr, timeout, cnt_max);
}

int LGSbus::poll()
{
  // Example:
  // # 1 1 4 3 1 2 3 4 19
  // # 1 1 16 2 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 1600 1740
    
  if (SR->find('#'))
  {
    int sum_cal, sum_ret, data[DATA_SUBBUF_SIZE];
    
    // (1) get data   
    int id      = SR->parseInt();
    int type    = SR->parseInt();
    int len     = SR->parseInt();
    int addr    = SR->parseInt();
    len = constrain(len, 1, DATA_SUBBUF_SIZE);
    sum_cal = id + type + len + addr;
    for (int i=0; i<len; i++)
    {
      data[i] = SR->parseInt();
      sum_cal += data[i]; 
    }
    int sum = SR->parseInt();
    
    // (2) put data into the buffer
    if (id==ID && sum_cal==sum)
    {
      addr = constrain(addr, 0, DATA_BUF_SIZE-1);
      LAST_ADDR = addr;
      if (type == WRITE_TYPE) 
      {
        for (int i=0; i<len; i++)
        {
          DATA_BUF[addr][i] = data[i];
        }
      }

      // (3) send return packet
      delay(retDelayTime);
      SR->write('&');
      SR->println(id);
      SR->println(RETURN_TYPE);
      SR->println(len);
      SR->println(addr);
      sum_ret = id + RETURN_TYPE + len + addr;
      for (int i=0; i<len; i++)
      {
        SR->println(DATA_BUF[addr][i]);
        sum_ret += DATA_BUF[addr][i];
      }
      SR->println(sum_ret);  
      return 1;
    }
  }
  return 0; // not responde or packet incorrect
}

int LGSbus::readLastAddress()
{
  return LAST_ADDR;
}