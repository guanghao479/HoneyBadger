using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace SyncClient
{
    public partial class Form1 : Form
    {
        private Watcher filesysWatcher;
        private NetClient netClient;
        private Process proc = new Process();
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void btn_move_Click(object sender, EventArgs e)
        {
            string path = this.txt_location.Text.Trim();
            if ((path == null) || (path == ""))
            {
                MessageBox.Show("location is empty!");
                return;
            }
            filesysWatcher = new Watcher(path, proc, this);
        }
        public void addFileChange(string ch)
        {
            this.txt_changes.Text += ch + "\n";
        }

        private void btn_clear_Click(object sender, EventArgs e)
        {
            this.txt_changes.Text = "";
        }

        private void btn_connect_Click(object sender, EventArgs e)
        {
            string ip = this.txt_ip.Text.Trim();
            if ((ip == null) || (ip == ""))
            {
                MessageBox.Show("IP is empty!");
                return;
            }
            string strport = this.txt_port.Text.Trim();
            if ((strport == null) || (strport == ""))
            {
                MessageBox.Show("Port is empty!");
                return;
            }
            netClient = new NetClient(ip, Int32.Parse(strport));
            netClient.start();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            netClient.write(
                Encoding.UTF8.GetBytes("abc"));
        }

        private void btn_register_Click(object sender, EventArgs e)
        {
            string name = this.txt_name.Text.Trim();
            if ((name == null) || (name == ""))
            {
                MessageBox.Show("name is empty!");
                return;
            }
            string passwd = this.txt_passwd.Text.Trim();
            if ((passwd == null) || (passwd == ""))
            {
                MessageBox.Show("password is empty!");
                return;
            }
            try
            {
                byte[] data = XmlClass.getRegisterMsg(ref name, ref passwd);
                if (data != null)
                {
                    netClient.write(data);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
    }
}
