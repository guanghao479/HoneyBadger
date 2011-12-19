using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

namespace SyncClient
{
    class RsyncDll
    {
        const string sigfile = "E:\\sigfile.temp";
        const string deltafile = "E:\\deltafile.temp";
        const string patchfile = "E:\\patchfile.temp";
        [DllImport("LibRsyncDll.dll", EntryPoint = "rsync_sig")]
        public static extern bool rsync_sig(string oldfile, string sigfile);
        [DllImport("LibRsyncDll.dll", EntryPoint = "rsync_delta")]
        public static extern bool rsync_delta(string newfile, string sigfile, string deltafile);
        [DllImport("LibRsyncDll.dll", EntryPoint = "rsync_patch")]
        public static extern bool rsync_patch(string oldfile, string newfile, string deltafile);

        public static Byte[] getFileSig(string oldfile)
        {
            FileStream fs = File.Create(sigfile);
            fs.Close();
            Byte[] sig = null;
            if (rsync_sig(oldfile, sigfile))
            {
                sig = File.ReadAllBytes(sigfile);
            }
            File.Delete(sigfile);
            return sig;
        }

        public static Byte[] getFileDelta(string newfile, Byte[] sig)
        {
            Byte[] delta = null;
            FileStream fs = File.Create(deltafile);
            fs.Close();
            File.WriteAllBytes(sigfile, sig);
            if (rsync_delta(newfile, sigfile, deltafile))
            {
                delta = File.ReadAllBytes(deltafile);
            }
            File.Delete(sigfile);
            File.Delete(deltafile);
            return delta;
        }

        public static void patchFile(string pfile, Byte[] delta)
        {
            FileStream fs = File.Create(patchfile);
            fs.Close();
            File.WriteAllBytes(deltafile, delta);
            if (rsync_patch(pfile, patchfile, deltafile))
            {
                File.Delete(pfile);
                File.Move(patchfile, pfile);
            }
            File.Delete(deltafile);
            File.Delete(patchfile);
        }
    }
}
