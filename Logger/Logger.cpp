
#include "Logger.hpp"

// Initialize static members
Logger::Level Logger::currentLevel = Logger::DEBUG;
std::ostream* Logger::output = &std::cout;
std::ofstream Logger::logFile;
bool Logger::isStandardOutput = true;
std::string Logger::format = "[%TIME%] [%LEVEL%] %MESSAGE%";

std::string Logger::intToString(int number)
{
    std::ostringstream oss;
    oss << number;
    return oss.str();
}

void Logger::setLevel(Level level)
{
    currentLevel = level;
}

void Logger::setOutput(std::ostream& outputStream)
{
    output = &outputStream;
    isStandardOutput = (output == &std::cout || output == &std::cerr);
}

void Logger::setFormat(const std::string& formatString)
{
    format = formatString;
}

std::string Logger::levelToString(Level level)
{
    if (isStandardOutput)
    {
        switch (level)
        {
            case DEBUG: return "\033[94m [DEBUG]"; // Light Blue
            case INFO:  return "\033[92m [INFO ]"; // Light Green
            case WARN:  return "\033[93m [WARN ]"; // Light Yellow
            case ERROR: return "\033[91m [ERROR]"; // Light Red
            case FATAL: return "\033[95m [FATAL]"; // Light Magenta
            default:    return "\033[97m [UNKNOWN]"; // Light White
        }
    }
    else
    {
        switch (level)
        {
            case DEBUG: return " [DEBUG]";
            case INFO:  return " [INFO ]";
            case WARN:  return " [WARN ]";
            case ERROR: return " [ERROR]";
            case FATAL: return " [FATAL]";
            default:    return " [UNKNOWN]";
        }
    }
}

std::string Logger::getCurrentTimeFormatted()
{
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%Y/%m/%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

void Logger::init(Logger::Level logLevel, const std::string& logFilePath)
{
    currentLevel = logLevel;
    
    std::cout << "\n\033[0;92m🤠 Ranchero is running.\033[0m\n" 
              << "\n🗃️  You can find logs at: \033[1;94mlogs/webserv.log\033[0m" 
              << std::endl;
              
    if (!logFilePath.empty())
    {
        logFile.open(logFilePath.c_str()); // C++98 requires c_str()
        if (logFile.is_open())
        {
            setOutput(logFile);
        }
    }
}

void Logger::cleanup()
{
    if (logFile.is_open())
    {
        logFile.close();
    }
}

void Logger::log(Level level, const std::string& message, const std::string& source)
{
    if (level < currentLevel) return;

    std::string timeStr = getCurrentTimeFormatted();
    std::string levelStr = levelToString(level);
    
    // Using string concatenation instead of ostringstream for C++98 compatibility
    std::string logLine = timeStr + levelStr + " [" + source + "] " + message;
    
    if (isStandardOutput)
        logLine += "\033[0m";

    (*output) << logLine << std::endl;
}
