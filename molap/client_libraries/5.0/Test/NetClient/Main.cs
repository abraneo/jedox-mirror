using System;
using Jedox.Palo.Comm;

namespace NetClient
{
    /// <summary>
    /// Main Class
    /// </summary>
    class MainClass
    {
        private const string SERVER_HOSTNAME = "localhost";
        private const string SERVER_PORT = "9777";
        private const string USERNAME = "admin";
        private const string PASSWORD = "admin";

        private static void TestPing(Connection c)
        {
            Console.Write("Ping...");
            c.Ping();
            Console.WriteLine("done.\n");
        }

        private static void TestRootListDatabases(Connection c)
        {
            Console.Write("RootListDatabases: ");
            String[] sa = c.RootListDatabases();
            Console.WriteLine("done.");
            Console.WriteLine("Databases:");
            foreach (String str in sa)
            {
                Console.WriteLine("\t" + str);
            }
            sa = c.RootListDatabases(DatabaseType.SystemDatabase);
            Console.WriteLine("SystemDatabases:");
            foreach (String str in sa) 
            {
                Console.WriteLine("\t" + str);
            }
            Console.WriteLine("End.\n");
        }

        [STAThread]
        static void Main(string[] args)
        {
            Connection c = null;
            Console.WriteLine("Press return to start...\n");
            Console.ReadLine();
            string path = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);

            path += @"\client.pem";
            Connection.InitSSL(path); 

            Console.Write("Connecting...");

            try
            {
                c = new Connection(SERVER_HOSTNAME, SERVER_PORT, USERNAME, PASSWORD);
                Console.WriteLine("done.\n");
                TestPing(c);

                TestRootListDatabases(c);

                Console.Write("Disconnecting...");
                c.Dispose();
                Console.WriteLine("done.\n");
                c = null;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Exception occured : " + ex.Message + "\n" + ex.StackTrace);
            }
            
            Console.WriteLine("Press return to exit...\n");
            Console.ReadLine();
        }
    }
}
