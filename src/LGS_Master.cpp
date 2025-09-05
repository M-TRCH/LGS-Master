
#include "LGS_Master.h"
#include "logger.h"

LGSbus bus;

void commu_init()
{ 
  bus.begin(bus.ID_MASTER, &RS485, RS485_BAUDRATE, RS485_TIMEOUT);
  RS485.receive();
}

void setInfo(int cl, int ch, int dd, int mm, int yy)
{
  bus.writeData(ADDR_GREET, 0, cl);
  bus.writeData(ADDR_GREET, 1, ch);
  bus.writeData(ADDR_GREET, 2, dd);
  bus.writeData(ADDR_GREET, 3, mm);
  bus.writeData(ADDR_GREET, 4, yy);
  bus.write(99, 5, ADDR_GREET, 50, 2);  
}

int moduleCheck(int row, int col)
{
  const int id = (row * 10) + col;
  return bus.read(id, ADDR_GREET, 200, 3);
}

void std_moduleCheck()
{
  LOG_INFO_MSG(CAT_LGS, "Starting module check");
  for (int r=8; r>=1; r--)
  {
    String row_status = "Row " + String(r) + ": ";
    for (int c=1; c<=8; c++)
    {
      row_status += String(moduleCheck(r, c));
      if (r == 1 && c >= 6)  break;
      if (c != 8) row_status += ", ";
    }
    LOG_DEBUG_MSG(CAT_LGS, row_status);
  }
}

int std_setColor(int id, int addr, int r, int g, int b, float f)
{
  bus.writeData(addr, 0, r * f);
  bus.writeData(addr, 1, g * f);
  bus.writeData(addr, 2, b * f);
  return bus.write(id, 3, addr, 50, 3); // addr num: 3, timeout: 50, count: 3
}

int std_setColor(int row, int col, int cl, bool state)
{
  const float factor = 1.0;
  const int id = (row * 10) + col;
  int ret = 0;
  
  switch (cl)
  {
    case 1:   // red
      if (state)  ret = std_setColor(id, 9, 255, 0, 0, factor);     
      else        ret = std_setColor(id, 9, 0, 0, 0, factor);
      break;

    case 2:   // green
      if (state)  ret = std_setColor(id, 10, 1, 255, 0, factor);     
      else        ret = std_setColor(id, 10, 0, 0, 0, factor);
      break;

    case 3:   // blue
      if (state)  ret = std_setColor(id, 11, 1, 0, 255, factor);     
      else        ret = std_setColor(id, 11, 0, 0, 0, factor);  
      break;

    case 4:   // yellow
      if (state)  ret = std_setColor(id, 12, 255, 145, 0, factor);  // calibated
      else        ret = std_setColor(id, 12, 0, 0, 0, factor);  
      break;
  }
  return ret;
}

int std_requestStatus(int row, int col, int cl_in)
{
  int ret = 0;
  const int id = (row * 10) + col;
  const int cl = constrain(cl_in, 1, 4) + 8;  
  //  cl = cl_in + 8
  //  cl -> 9:red, 10:green, 11:blue, 12:yellow 
  
  if (bus.read(id, cl, 200, 3))
  {
    if (bus.readData(cl, 0) == 0) ret = 1;
    else  ret = 2;
  }
  else
    ret = 0;
    
  // ret = 0: error, 1: idle, 2: busy
  return ret;  
}

void nct_moduleCheck()
{
  LOG_INFO_MSG(CAT_LGS, "Starting NCT module check");
  for (int r=9; r>=0; r--)
  {
    String row_status = "Row " + String(r) + ": ";
    for (int c=1; c<=8; c++)
    {
      row_status += String(moduleCheck(r, c));
      if (c != 8) row_status += ", ";
    }
    LOG_DEBUG_MSG(CAT_LGS, row_status);
  }
}

int nct_setActive(int row, int col, int cl, int bn, int num)
{
  int ret = 0, ret2 = 0;
  const int id = (row * 10) + col;
 
  if (cl == 0)  // off
  {
    bus.writeData(ADDR_GREET, 0, 0);    
    bus.writeData(ADDR_CL_LED1, 0, 0);
    ret2 = bus.read(id, ADDR_GREET, 200, 3); 
    if (ret2 && bus.readData(ADDR_GREET, 0))  // sensor check    
    {
      ret2 += bus.write(id, 1, ADDR_CL_LED1, 200, 3);  // send deactivate 
    }   
  }
  else
  {
    bus.writeData(ADDR_CL_LED1, 0, cl);
    bus.writeData(ADDR_CL_LED1, 1, bn);
    bus.writeData(ADDR_CL_LED1, 2, num);
    ret = bus.write(id, 3, ADDR_CL_LED1, 200, 3);  
  }
  // ret = 0: error, 1: success
  // ret2 = 0: error, 1: success only request, 2: full success
  return ret + ret2;
}

void nct_allUnlock()
{
  LOG_INFO_MSG(CAT_LGS, "Starting all unlock sequence");
  for (int r=9; r>=0; r--)
  {
    String row_status = "Row " + String(r) + " unlock: ";
    for (int c=1; c<=8; c++)
    {
      int result = nct_setActive(r, c, 1, 5, 0);
      row_status += String(result);
      if (c != 8) row_status += ", ";
      delay(800);
    }
    LOG_DEBUG_MSG(CAT_LGS, row_status);
  }  
}

void nct_allLock()
{
  LOG_INFO_MSG(CAT_LGS, "Starting all lock sequence");
  for (int r=9; r>=0; r--)
  {
    String row_status = "Row " + String(r) + " lock: ";
    for (int c=1; c<=8; c++)
    {
      int result = nct_setActive(r, c);
      row_status += String(result);
      if (c != 8) row_status += ", ";
      delay(800);
    }
    LOG_DEBUG_MSG(CAT_LGS, row_status);
  }  
}

int nct_requestStatus(int row, int col)
{
  int ret = 0;
  const int id = (row * 10) + col;
  
  if (bus.read(id, ADDR_CL_LED1, 200, 3))
  {
    if (bus.readData(ADDR_CL_LED1, 0) == 0) ret = 1;
    else  ret = 2;
  }
  else
    ret = 0;
    
  // ret = 0: error, 1: idle, 2: busy
  return ret;
}
