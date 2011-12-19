using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading;


namespace SyncClient
{
    class FileEvent
    {
        public FileEvent(Int64 num, WatcherChangeTypes type, string path, string oldpath)
        {
            eventnum = num;
            eventtype = type;
            eventpath = path;
            oldevpath = oldpath;
        }
        public FileEvent(Int64 num, WatcherChangeTypes type, string path)
        {
            eventnum = num;
            eventtype = type;
            eventpath = path;
        }
        public Int64 eventnum;
        public WatcherChangeTypes eventtype;
        public string eventpath;
        public string oldevpath;
    }
    class Process
    {
        private Queue<FileEvent> eventQueue = new Queue<FileEvent>();
        private AutoResetEvent cond = new AutoResetEvent(false);
        private SortedDictionary<Int64, FileEvent> eventTable = new SortedDictionary<Int64, FileEvent>();
        private Thread eventProcThread;
        public Process()
        {
            eventProcThread = new Thread(OnFileChange);
        }
        public void enqueueEvent(ref FileEvent fe)
        {
            lock (eventQueue)
            {
                eventQueue.Enqueue(fe);
            }
            cond.Set();
        }
        public FileEvent dequeueEvent()
        {
            FileEvent fe;
            lock (eventQueue)
            {
                if (eventQueue.Count == 0)
                    fe = null;
                else
                    fe = eventQueue.Dequeue();
            }
            return fe;
        }
        public void addevent(ref FileEvent fe)
        {
            lock (eventTable)
            {
                eventTable.Add(fe.eventnum, fe);
            }
        }
        public FileEvent getevent(Int64 num)
        {
            lock (eventTable)
            {
                if (eventTable.ContainsKey(num))
                    return eventTable[num];
                else
                    return null;
            }
        }
        public void rmevent(Int64 num)
        {
            lock (eventTable)
            {
                eventTable.Remove(num);
            }
        }
        private void OnFileChange()
        {
            FileEvent fe = null;
            while ((fe = eventQueue.Dequeue()) == null) {
                cond.WaitOne();
            }
            switch (fe.eventtype) { 
                case WatcherChangeTypes.Changed:
                    break;
                case WatcherChangeTypes.Created:
                    break;
                case WatcherChangeTypes.Deleted:
                    break;
            }
        }
    }
}
