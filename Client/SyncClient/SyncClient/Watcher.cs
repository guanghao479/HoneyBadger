using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Security.Permissions;
using System.IO;

namespace SyncClient
{
    class Watcher
    {
        private string pathWatched;
        private Process proc;
        private Int64 eventnum = 0;
        private Form1 mainForm;
        public Watcher(string path, Process proc, Form1 form)
        {
            pathWatched = path;
            this.proc = proc;
            mainForm = form;
            Run();
        }

        [PermissionSet(SecurityAction.Demand, Name = "FullTrust")]
        private void Run()
        {
            if ((pathWatched == null) || (pathWatched == ""))
            {
                return;
            }

            // Create a new FileSystemWatcher and set its properties.
            FileSystemWatcher watcher = new FileSystemWatcher();
            watcher.Path = pathWatched;
            /* Watch for changes in LastAccess and LastWrite times, and
               the renaming of files or directories. */
            watcher.NotifyFilter = NotifyFilters.LastWrite
               | NotifyFilters.FileName | NotifyFilters.DirectoryName;
            watcher.IncludeSubdirectories = true;
            // Only watch text files.
            watcher.Filter = "*.*";

            // Add event handlers.
            watcher.Changed += new FileSystemEventHandler(OnChanged);
            watcher.Created += new FileSystemEventHandler(OnCreated);
            watcher.Deleted += new FileSystemEventHandler(OnDeleted);
            watcher.Renamed += new RenamedEventHandler(OnRenamed);

            // Begin watching.
            watcher.EnableRaisingEvents = true;
        }

        private void OnChanged(object source, FileSystemEventArgs e)
        {
            mainForm.addFileChange("File: " + e.FullPath + " " + e.ChangeType);
            FileAttributes fa = File.GetAttributes(e.FullPath);
            if (fa == FileAttributes.Directory)
            {
                /*文件夹修改不处理*/
                return;
            }
            FileEvent fe = new FileEvent(++eventnum, e.ChangeType, e.FullPath);
            proc.addevent(ref fe);
        }

        private void OnCreated(object source, FileSystemEventArgs e)
        {
            mainForm.addFileChange("File: " + e.FullPath + " " + e.ChangeType);
            FileAttributes fa = File.GetAttributes(e.FullPath);
            if (fa == FileAttributes.Directory)
            {
                /*文件夹新建时，所有子文件也要生成*/
                recurAddevent(e.FullPath, e.ChangeType);
            }
            FileEvent fe = new FileEvent(++eventnum, e.ChangeType, e.FullPath);
            proc.addevent(ref fe);
        }

        private void OnDeleted(object source, FileSystemEventArgs e)
        {
            OnCreated(source, e);
        }

        private void OnRenamed(object source, RenamedEventArgs e)
        {
            // Specify what is done when a file is renamed.
            mainForm.addFileChange("File: " + e.OldFullPath + " renamed to " + e.FullPath);
            FileEvent fe = new FileEvent(++eventnum, e.ChangeType, e.FullPath, e.OldFullPath);
            proc.addevent(ref fe);
        }

        private void recurAddevent(string dir, WatcherChangeTypes changetype)
        {
            string[] filelist = Directory.GetFileSystemEntries(dir);
            foreach (string file in filelist)
            {
                if (Directory.Exists(file))
                {
                    recurAddevent(file, changetype);
                }
                else
                {
                    FileEvent fe = new FileEvent(++eventnum, changetype, file);
                    proc.addevent(ref fe);
                }
            }
        }

    }
}
