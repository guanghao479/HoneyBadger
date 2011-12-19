namespace SyncClient
{
    partial class Form1
    {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.txt_location = new System.Windows.Forms.TextBox();
            this.btn_move = new System.Windows.Forms.Button();
            this.txt_changes = new System.Windows.Forms.TextBox();
            this.btn_clear = new System.Windows.Forms.Button();
            this.gpb_server = new System.Windows.Forms.GroupBox();
            this.btn_connect = new System.Windows.Forms.Button();
            this.txt_port = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.txt_ip = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.btn_sendmsg = new System.Windows.Forms.Button();
            this.gpb_location = new System.Windows.Forms.GroupBox();
            this.gpb_register = new System.Windows.Forms.GroupBox();
            this.btn_register = new System.Windows.Forms.Button();
            this.txt_passwd = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txt_name = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.button1 = new System.Windows.Forms.Button();
            this.gpb_server.SuspendLayout();
            this.gpb_register.SuspendLayout();
            this.SuspendLayout();
            // 
            // txt_location
            // 
            this.txt_location.Location = new System.Drawing.Point(37, 30);
            this.txt_location.Name = "txt_location";
            this.txt_location.Size = new System.Drawing.Size(241, 21);
            this.txt_location.TabIndex = 1;
            this.txt_location.Text = "C:\\Users\\Samlou\\Dropbox";
            // 
            // btn_move
            // 
            this.btn_move.Location = new System.Drawing.Point(297, 30);
            this.btn_move.Name = "btn_move";
            this.btn_move.Size = new System.Drawing.Size(75, 23);
            this.btn_move.TabIndex = 2;
            this.btn_move.Text = "move...";
            this.btn_move.UseVisualStyleBackColor = true;
            this.btn_move.Click += new System.EventHandler(this.btn_move_Click);
            // 
            // txt_changes
            // 
            this.txt_changes.Location = new System.Drawing.Point(12, 233);
            this.txt_changes.Multiline = true;
            this.txt_changes.Name = "txt_changes";
            this.txt_changes.Size = new System.Drawing.Size(343, 258);
            this.txt_changes.TabIndex = 3;
            // 
            // btn_clear
            // 
            this.btn_clear.Location = new System.Drawing.Point(361, 233);
            this.btn_clear.Name = "btn_clear";
            this.btn_clear.Size = new System.Drawing.Size(75, 22);
            this.btn_clear.TabIndex = 4;
            this.btn_clear.Text = "clear";
            this.btn_clear.UseVisualStyleBackColor = true;
            this.btn_clear.Click += new System.EventHandler(this.btn_clear_Click);
            // 
            // gpb_server
            // 
            this.gpb_server.Controls.Add(this.btn_connect);
            this.gpb_server.Controls.Add(this.txt_port);
            this.gpb_server.Controls.Add(this.label2);
            this.gpb_server.Controls.Add(this.txt_ip);
            this.gpb_server.Controls.Add(this.label1);
            this.gpb_server.Location = new System.Drawing.Point(12, 70);
            this.gpb_server.Name = "gpb_server";
            this.gpb_server.Size = new System.Drawing.Size(444, 48);
            this.gpb_server.TabIndex = 6;
            this.gpb_server.TabStop = false;
            this.gpb_server.Text = "Server";
            // 
            // btn_connect
            // 
            this.btn_connect.Location = new System.Drawing.Point(349, 17);
            this.btn_connect.Name = "btn_connect";
            this.btn_connect.Size = new System.Drawing.Size(75, 23);
            this.btn_connect.TabIndex = 4;
            this.btn_connect.Text = "connect";
            this.btn_connect.UseVisualStyleBackColor = true;
            this.btn_connect.Click += new System.EventHandler(this.btn_connect_Click);
            // 
            // txt_port
            // 
            this.txt_port.Location = new System.Drawing.Point(256, 15);
            this.txt_port.Name = "txt_port";
            this.txt_port.Size = new System.Drawing.Size(64, 21);
            this.txt_port.TabIndex = 3;
            this.txt_port.Text = "3432";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(214, 18);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(35, 12);
            this.label2.TabIndex = 2;
            this.label2.Text = "Port:";
            // 
            // txt_ip
            // 
            this.txt_ip.Location = new System.Drawing.Point(48, 14);
            this.txt_ip.Name = "txt_ip";
            this.txt_ip.Size = new System.Drawing.Size(142, 21);
            this.txt_ip.TabIndex = 1;
            this.txt_ip.Text = "127.0.0.1";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(20, 17);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(23, 12);
            this.label1.TabIndex = 0;
            this.label1.Text = "IP:";
            // 
            // btn_sendmsg
            // 
            this.btn_sendmsg.Location = new System.Drawing.Point(361, 285);
            this.btn_sendmsg.Name = "btn_sendmsg";
            this.btn_sendmsg.Size = new System.Drawing.Size(75, 25);
            this.btn_sendmsg.TabIndex = 7;
            this.btn_sendmsg.Text = "sendMsg";
            this.btn_sendmsg.UseVisualStyleBackColor = true;
            this.btn_sendmsg.Click += new System.EventHandler(this.button1_Click);
            // 
            // gpb_location
            // 
            this.gpb_location.Location = new System.Drawing.Point(12, 12);
            this.gpb_location.Name = "gpb_location";
            this.gpb_location.Size = new System.Drawing.Size(444, 52);
            this.gpb_location.TabIndex = 8;
            this.gpb_location.TabStop = false;
            this.gpb_location.Text = "Sync location";
            // 
            // gpb_register
            // 
            this.gpb_register.Controls.Add(this.btn_register);
            this.gpb_register.Controls.Add(this.txt_passwd);
            this.gpb_register.Controls.Add(this.label3);
            this.gpb_register.Controls.Add(this.txt_name);
            this.gpb_register.Controls.Add(this.label4);
            this.gpb_register.Location = new System.Drawing.Point(12, 126);
            this.gpb_register.Name = "gpb_register";
            this.gpb_register.Size = new System.Drawing.Size(444, 48);
            this.gpb_register.TabIndex = 9;
            this.gpb_register.TabStop = false;
            this.gpb_register.Text = "Register";
            // 
            // btn_register
            // 
            this.btn_register.Location = new System.Drawing.Point(349, 18);
            this.btn_register.Name = "btn_register";
            this.btn_register.Size = new System.Drawing.Size(75, 23);
            this.btn_register.TabIndex = 9;
            this.btn_register.Text = "register";
            this.btn_register.UseVisualStyleBackColor = true;
            this.btn_register.Click += new System.EventHandler(this.btn_register_Click);
            // 
            // txt_passwd
            // 
            this.txt_passwd.Location = new System.Drawing.Point(273, 16);
            this.txt_passwd.Name = "txt_passwd";
            this.txt_passwd.Size = new System.Drawing.Size(64, 21);
            this.txt_passwd.TabIndex = 8;
            this.txt_passwd.Text = "3432";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(214, 19);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(59, 12);
            this.label3.TabIndex = 7;
            this.label3.Text = "Password:";
            // 
            // txt_name
            // 
            this.txt_name.Location = new System.Drawing.Point(58, 15);
            this.txt_name.Name = "txt_name";
            this.txt_name.Size = new System.Drawing.Size(132, 21);
            this.txt_name.TabIndex = 6;
            this.txt_name.Text = "Sam";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(20, 18);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(35, 12);
            this.label4.TabIndex = 5;
            this.label4.Text = "Name:";
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(361, 345);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 10;
            this.button1.Text = "testasync";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click_1);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(468, 506);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.gpb_register);
            this.Controls.Add(this.btn_sendmsg);
            this.Controls.Add(this.gpb_server);
            this.Controls.Add(this.btn_clear);
            this.Controls.Add(this.txt_changes);
            this.Controls.Add(this.btn_move);
            this.Controls.Add(this.txt_location);
            this.Controls.Add(this.gpb_location);
            this.Name = "Form1";
            this.Text = "Preferences";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.gpb_server.ResumeLayout(false);
            this.gpb_server.PerformLayout();
            this.gpb_register.ResumeLayout(false);
            this.gpb_register.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox txt_location;
        private System.Windows.Forms.Button btn_move;
        private System.Windows.Forms.TextBox txt_changes;
        private System.Windows.Forms.Button btn_clear;
        private System.Windows.Forms.GroupBox gpb_server;
        private System.Windows.Forms.Button btn_connect;
        private System.Windows.Forms.TextBox txt_port;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox txt_ip;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button btn_sendmsg;
        private System.Windows.Forms.GroupBox gpb_location;
        private System.Windows.Forms.GroupBox gpb_register;
        private System.Windows.Forms.Button btn_register;
        private System.Windows.Forms.TextBox txt_passwd;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txt_name;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button button1;
    }
}

