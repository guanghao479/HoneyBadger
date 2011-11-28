using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace SyncClient
{
    class XmlClass
    {
        public void readXmlFile() {
            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml("test.xml");

        }
    }
}
