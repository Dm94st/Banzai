#include <iostream>
#include <vector>
#include <fstream>

class CSVReader
{
public:
  CSVReader() = default;
  CSVReader(const std::string &filePath, const char &delimiter = '\0'):
      m_filePath(filePath), m_delimiter(delimiter)
  {
    FetchData();
  }
  ~CSVReader()
  {}

  static void WriteInFile(const std::string &outPath, const std::vector<double> &interpValues)
  {
    std::ofstream outFile(outPath);
    if(outFile.is_open())
    {
      for(auto it = interpValues.begin(); it != interpValues.end(); ++it)
      {
        outFile <<*it<<std::endl;
      }
      outFile.close();
    }
  }

  void Print() const
  {
    for(auto line:m_data)
    {
      std::cout<<line<<" "<<std::endl;
    }
  }

  std::vector<std::string> GetData() const
  {
    return m_data;
  }

  char GetDelimiter() const
  {
    return m_delimiter;
  }

private:
  const std::string m_filePath;
  const char m_delimiter;
  std::vector<std::string> m_data;

  void FetchData()
  {
    std::fstream m_file(m_filePath);
    std::string curLine;
    if(m_file.is_open())
    {
      while(std::getline(m_file, curLine))
      {
        m_data.push_back(curLine);
      }
      m_file.close();
    }
    else
    {
      throw std::invalid_argument("Unable to open file");
    }
  }
}
;

class Interpolator
{
public:
  Interpolator(const CSVReader &srcData, const CSVReader &srcArgs, const std::string &outPath):
    m_srcData(srcData), m_srcArgs(srcArgs), m_outPath(outPath)
  {}
  virtual ~Interpolator(){}

  virtual double InterpolateValue(double x) = 0;

  void InterpolateAll()
  {
    std::vector<std::string> srcArgs = m_srcArgs.GetData();
    std::vector<std::string> srcData =  m_srcData.GetData();

    double lowBound = std::stod(srcData.front()); // Lowest value of X
    double hiBound = std::stod(srcData.back()); // Highest value of X

    for(auto it = srcArgs.begin(); it != srcArgs.end(); ++it)
    {
      double curValue = std::stod(*it);

      // If current value in avialible range
      if( curValue > lowBound && curValue < hiBound)
      {
        double interpValue = InterpolateValue(curValue);
        m_interpValues.push_back(interpValue);
        std::cout<<interpValue<<std::endl;
      }
      else
        continue;
    }
    CSVReader::WriteInFile(m_outPath, m_interpValues);
  }
protected:
  CSVReader m_srcData; // With delimiters
  CSVReader m_srcArgs; // Without delimiters
  const std::string m_outPath; // Path until output file
  std::vector<double> m_interpValues; // Array with interpolated values

  // Find nearest index of row
  int IdxOfNearestRow(double x)
  {
    std::vector<std::string> srcXY = m_srcData.GetData(); // Source data
    std::vector<double> curValue, nextValue; // Source X and Y of current row
    int desiredIdx = 0; // Index of row
    int idx = 0; // Index

    for(auto it = srcXY.begin(); it != srcXY.end() - 1; ++it, ++idx)
    {
      curValue = SplitByDelimiter(*it);
      nextValue = SplitByDelimiter(*(std::next(it)));
      if(x >= curValue.front() && x <= nextValue.front())
      {
        desiredIdx = std::distance(srcXY.begin(), it);
      }
    }
    return desiredIdx;
  }

  // Split each row by delimiter
  std::vector<double> SplitByDelimiter(const std::string &srcStr)
  {
    std::vector<double> splittedStr;
    std::string curStrValue; // Current value in string

    for(auto it = srcStr.begin(); it != srcStr.end(); ++it)
    {
      if(*it != m_srcData.GetDelimiter())
      {
        curStrValue += *it;
      }
      else if(*it == m_srcData.GetDelimiter())
      {
        double curValue = std::stod(curStrValue);
        splittedStr.push_back(curValue);
        curStrValue.clear();
      }
      else
         continue;
    }
    splittedStr.push_back(std::stod(curStrValue)); // Add last value
    return splittedStr;
  }
}
;
class NearestNeighbor: public Interpolator
{
public:
  using Interpolator::Interpolator; // Inherit constructor from base class

private:
  double InterpolateValue(double x) override
  {
    std::vector<std::string> srcXY = m_srcData.GetData();
    int desiredInterval = IdxOfNearestRow(x);
    double interpolatedValue = SplitByDelimiter(srcXY[desiredInterval]).back();
    return interpolatedValue;
  }
}
;
class Linear: public Interpolator
{
public:
  using Interpolator::Interpolator;

private:
  double InterpolateValue(double x) override
  {
    std::vector<std::string> srcXY = m_srcData.GetData();
    int desiredInterval = IdxOfNearestRow(x);

    double xVal = SplitByDelimiter(srcXY[desiredInterval]).front(); // X_n
    double yVal = SplitByDelimiter(srcXY[desiredInterval]).back(); // Y_n

    double xValNext = SplitByDelimiter(srcXY[desiredInterval + 1]).front(); // X_n+1
    double yValNext = SplitByDelimiter(srcXY[desiredInterval + 1]).back(); // Y_n+1

    double interpolatedValue = yVal + (yValNext - yVal) * (x - xVal)/(xValNext - xVal);
    return interpolatedValue;
  }
}
;
class Quadratic: public Interpolator
{
public:
  using Interpolator::Interpolator;

private:
  double InterpolateValue(double x) override
  {
    /*
    Formula:
    y = y1*(x-x2)*(x-x3)/(x1-x2)(x1-x3) +
    y2*(x-x1)*(x-x3)/(x2-x1)*(x2-x3) +
    y3*(x-x1)*(x-x2)/(x3-x1)*(x3-x2)
    */

    std::vector<std::string> srcXY = m_srcData.GetData();
    int desiredIdx = IdxOfNearestRow(x);

    double xValPrev = SplitByDelimiter(srcXY[desiredIdx - 1]).front(); // X_n-1
    double yValPrev = SplitByDelimiter(srcXY[desiredIdx - 1]).back(); // Y_n-1

    double xVal = SplitByDelimiter(srcXY[desiredIdx]).front(); // X_n
    double yVal = SplitByDelimiter(srcXY[desiredIdx]).back(); // Y_n

    double xValNext = SplitByDelimiter(srcXY[desiredIdx + 1]).front(); // X_n+1
    double yValNext = SplitByDelimiter(srcXY[desiredIdx + 1]).back(); // Y_n+1

    double interpolatedValue = (yValPrev*((x-xVal)*(x-xValNext)))/((xValPrev-xVal)*(xValPrev-xValNext)) +\
                               (yVal*((x-xValPrev)*(x-xValNext)))/((xVal-xValPrev)*(xVal-xValNext)) +\
                               (yValNext*((x-xValPrev)*(x-xVal)))/((xValNext-xValPrev)*(xValNext-xVal));
    return interpolatedValue;
  }
}
;

enum InterpType
{
  NEAREST_NEIGHBOR = 1,
  LINEAR = 2,
  QUADRATIC = 3,
};

int main(int argc, char* argv[])
{
  std::string srcXYPath;
  char * delimiter;
  std::string srcArgsPath;
  std::string outFilePath;

  int interpolateMethod = 0;

  //Except four arguments
  if (argc < 4)
  {
    std::cerr << "Usage: " << argv[0] << "<Input file with X and Y values> <Input file with args> <Output File>"
              << std::endl;
    return 1;
  }
  else
  {
    for(int i = 1; i < argc; ++i)
    {
      srcXYPath = argv[i++]; // srcValues
      delimiter = argv[i++]; //delim
      srcArgsPath = argv[i++]; // srcArgs
      outFilePath = argv[i++]; // Out Path
    }
  }

  CSVReader srcXY(srcXYPath, *delimiter);
  CSVReader srcArgs(srcArgsPath);

  std::cout<<"srcXYPath: "  <<srcXYPath<<"\n" \
           <<"delim: "      <<delimiter<<"\n" \
           <<"srcArgsPath: "<<srcArgsPath<<"\n" \
           <<"outFilePath: "<<outFilePath<<"\n" \
           <<std::endl;

  bool isRunning = true;

  while(isRunning)
  {
    std::cout<<"Enter interpolate method"<<std::endl;
    std::cin >> interpolateMethod;
    try
    {
      switch(interpolateMethod)
      {
        case InterpType::LINEAR:
        {
          std::cout<<"You are using linear method"<<std::endl;
          auto linearMethod = std::make_shared<Linear>(srcXY,srcArgs,outFilePath);
          linearMethod->InterpolateAll();
          break;
        }
        case InterpType::NEAREST_NEIGHBOR:
        {
          std::cout<<"You are using Nearest Neighbor method"<<std::endl;
          auto nnMethod = std::make_shared<NearestNeighbor>(srcXY,srcArgs,outFilePath);
          nnMethod->InterpolateAll();
          break;
        }
        case InterpType::QUADRATIC:
        {
          std::cout<<"You are using quadratic method"<<std::endl;
          auto quadMethod = std::make_shared<Quadratic>(srcXY,srcArgs,outFilePath);
          quadMethod->InterpolateAll();
          break;
        }
        default:
        {
          isRunning = false;
          throw std::invalid_argument("Unknown method!");
          break;
        }
      }
    }
    catch(std::exception const &ex)
    {
      std::cout<<ex.what()<<std::endl;
    }
  }
  return 0;
}
