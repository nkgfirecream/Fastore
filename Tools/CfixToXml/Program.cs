using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

using CommandLine;

namespace CfixToXml
{
    class Formatter
    {
        [Argument(ArgumentType.Required, ShortName = "i", HelpText = "The path of the CFIX file to read.")]
        public string InputPath = String.Empty;

        [Argument(ArgumentType.Required, ShortName = "o", HelpText = "The path of the XML file to write.")]
        public string OutputPath = String.Empty;

        [Argument(ArgumentType.Required, ShortName = "n", HelpText = "The name of the test.")]
        public string Name = String.Empty;

        //TODO: configFile, time, skipped, and environment are hard-coded to match xsl.  Should probably build a custom xsl to match this xml instead.
        private const string ASSEMBLYTEMPLATE = "<assemblies><assembly name=\"{0}\" run-date=\"{1}\" run-time=\"{2}\" configFile=\"unknown\" time=\"1.0\" total=\"{3}\" passed=\"{4}\" failed=\"{5}\" skipped=\"0\" environment=\"32-bit .NET 4.0.30319.261\" test-framework=\"xUnit.net 1.8.0.1545\">{6}</assembly></assemblies>";      
        public int Run()
        {
            bool hasErrors = false;
            try
            {
                string input = String.Empty;

                using (StreamReader inputFile = new FileInfo(InputPath).OpenText())
                {
                    input = inputFile.ReadToEnd();
                }

                List<Test> tests = ParseTests(input);
                Dictionary<string, Class> classes = new Dictionary<string, Class>();
                foreach (Test test in tests)
                {
                    if (!classes.ContainsKey(test.Type))
                        classes.Add(test.Type, new Class() { Name = test.Type });
                    classes[test.Type].Tests.Add(test);
                }
                string classesText =  classes.Values.Select(ts => ts.ToString()).Aggregate((ts, tss) => ts + tss);
                string output = String.Format
                    (
                        ASSEMBLYTEMPLATE, 
                        Name, 
                        DateTime.Now.ToShortDateString(), 
                        DateTime.Now.ToShortTimeString(), 
                        classes.Sum(c => c.Value.Total).ToString(), 
                        classes.Sum(c => c.Value.Passed).ToString(), 
                        classes.Sum(c => c.Value.Failed).ToString(),
                       // input.Substring(0, input.IndexOf("\r\n")),
                        classesText
                    );
                using (FileStream outputFile = new FileInfo(OutputPath).OpenWrite())
                {
                    byte[] bytes = new byte[output.Length * sizeof(char)];
                    Buffer.BlockCopy(output.ToCharArray(), 0, bytes, 0, bytes.Length);
                    outputFile.Write(bytes, 0, bytes.Length);
                }
            }
            catch (Exception e)
            {
                hasErrors = true;
            }            

            if (hasErrors)
                return 1;
            return 0;
        }

        private List<Test> ParseTests(string input)
        {
            List<Test> result = new List<Test>();
            string fullName = String.Empty;
            string resultText = String.Empty;
            foreach (string test in input.Split(new char[] { '[' }))
            {
                if (test.IndexOf("]") < 0)
                    continue;
                resultText = test.Substring(0, test.IndexOf("]"));
                fullName = test.Substring(test.IndexOf("]") + 1).Trim();
                result.Add
                    (
                        new Test() 
                        { 
                            Name = fullName, 
                            Method = fullName.Substring(fullName.LastIndexOf(".") + 1), 
                            Type = fullName.Replace(fullName.Substring(fullName.LastIndexOf(".")), String.Empty),
                            ResultText = resultText,
                            Result = resultText == "Success" ? TestResult.Pass : TestResult.Fail
                        }
                    );
            }
            return result;
        }
     }

    enum TestResult
    {
        Pass,
        Fail
    }

    class Class
    {         
        private List<Test> tests = new List<Test>();
        public List<Test> Tests { get { return tests; } }        
        public string Name { get; set; }
        public int Total { get { return Passed + Failed; } }
        public int Passed { get { return tests.Count(t => t.Result == TestResult.Pass); } }
        public int Failed { get { return tests.Count(t => t.Result != TestResult.Pass); } }
        
        //TODO: time and skipped are hard-coded
        private const string CLASSTEMPLATE = "<class time=\"1.0\" name=\"{0}\" total=\"{1}\" passed=\"{2}\" failed=\"{3}\" skipped\"0\" >{4}</class>";
        public override string ToString()
        {
            return String.Format(CLASSTEMPLATE, Name, Total.ToString(), Passed.ToString(), Failed.ToString(), tests.Select(ts => ts.ToString()).Aggregate((ts, tss) => ts + tss));
        }
    }

    struct Test
    {
        //TODO: time is hard-coded, add resulttext back in
        private const string TESTTEMPLATE = "<test name=\"{0}\" type=\"{1}\" method=\"{2}\" result=\"{3}\" time=\"1.0\" />";
        public string Name { get; set; }
        public TestResult Result { get; set; }
        public String ResultText { get; set; }
        public string Method { get; set; }
        public string Type { get; set; }
        public override string ToString()
        {
            return String.Format(TESTTEMPLATE, Name, Type, Method, Enum.GetName(typeof(TestResult), Result));  
        }
    }

    class Program
    {
        static int Main(string[] args)
        {
            Formatter formatter = new Formatter();
            if (CommandLine.Parser.ParseArgumentsWithUsage(args, formatter))
#if DEBUG
                formatter.Run();
                Console.Read();
#else
               return formatter.Run();
#endif
            return 1;
        }
    }
}
