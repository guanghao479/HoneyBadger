using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.IO;
using System.Xml.Linq;
using System.Xml.Schema;

namespace SyncClient
{
    class XmlClass
    {
        public static byte[] getRegisterMsg(ref string name, ref string passwd)
        {
            registerMessageType msg = new registerMessageType();
            msg.userid = name;
            msg.password = passwd;
            return serializerXml(msg);
        }

        private static byte[] serializerXml(object obj)
        {
            MemoryStream ms = new MemoryStream();
            StreamWriter sw = new StreamWriter(ms);
            System.Xml.Serialization.XmlSerializer xs = new System.Xml.Serialization.XmlSerializer(obj.GetType());
            xs.Serialize(sw, obj);
            ms.Position = 0;
            byte[] data = ms.GetBuffer();
            if (validateXml(data))
            {
                return data;
            }
            else
            {
                return null;
            }
        }

        public static object deserializeXml(byte[] xmldata)
        {
            if (!validateXml(xmldata))
            {
                return null;
            }
            MemoryStream ms = new MemoryStream();
            ms.Read(xmldata, 0, xmldata.Length);
            XDocument doc = XDocument.Load(ms);
            string msgtype = doc.Root.Attribute("messageType").Value;
            object msg = null;
            switch (msgtype)
            {
                case "register":
                    msg = new registerMessageType();
                    break;
                case "login":
                    break;
                case "newfile":
                    break;
                case "filepatch":
                    break;
                case "filecontent":
                    break;
            }
            StreamReader sr = new StreamReader(ms);
            System.Xml.Serialization.XmlSerializer xs = new System.Xml.Serialization.XmlSerializer(msg.GetType());
            msg = xs.Deserialize(sr);
            return msg;
        }

        private static bool validateXml(byte[] xmldata)
        {
            MemoryStream ms = new MemoryStream();
            ms.Read(xmldata, 0, xmldata.Length);
            XDocument doc = XDocument.Load(ms);
            XmlSchemaSet schemas = new XmlSchemaSet();
            schemas.Add("", XmlReader.Create("honeybadger.xsd"));
            bool errors = false;
            string errinfo = "xml data error: ";
            doc.Validate(schemas, (o, e) =>
            {
                errinfo += e.Message + ";";
                errors = true;
            });
            if (errors)
            {
                throw new Exception(errinfo);
            }
            return !errors;
        }
    }
}
