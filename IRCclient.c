#include <cairo.h>
#include <gtk/gtk.h>
#include <time.h>
//#include <curses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

char * user;
char * password;
char * host;
char * sport;
int port;
char currentUser[100] = "baker";
char currentRoom[100] = "DEFAULT";
char userArray[100][100];
char passwordArray[100][100];
int n = 0;

int open_client_socket(char * host, int port) {
	// Initialize socket address structure
	struct  sockaddr_in socketAddress;

	// Clear sockaddr structure
	memset((char *)&socketAddress,0,sizeof(socketAddress));

	// Set family to Internet 
	socketAddress.sin_family = AF_INET;

	// Set port
	socketAddress.sin_port = htons((u_short)port);

	// Get host table entry for this host
	struct  hostent  *ptrh = gethostbyname(host);
	if ( ptrh == NULL ) {
		perror("gethostbyname");
		exit(1);
	}

	// Copy the host ip address to socket address structure
	memcpy(&socketAddress.sin_addr, ptrh->h_addr, ptrh->h_length);

	// Get TCP transport protocol entry
	struct  protoent *ptrp = getprotobyname("tcp");
	if ( ptrp == NULL ) {
		perror("getprotobyname");
		exit(1);
	}

	// Create a tcp socket
	int sock = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sock < 0) {
		perror("socket");
		exit(1);
	}

	// Connect the socket to the specified server
	if (connect(sock, (struct sockaddr *)&socketAddress,
				sizeof(socketAddress)) < 0) {
		perror("connect");
		exit(1);
	}

	return sock;
}

#define MAX_RESPONSE (10 * 1024)
int sendCommand(char *  host, int port, char * command, char * response) {

	int sock = open_client_socket( host, port);

	if (sock<0) {
		return 0;
	}

	// Send command
	write(sock, command, strlen(command));
	write(sock, "\r\n",2);

	//Print copy to stdout
	write(1, command, strlen(command));
	write(1, "\r\n",2);

	// Keep reading until connection is closed or MAX_REPONSE
	int n = 0;
	int len = 0;
	while ((n=read(sock, response+len, MAX_RESPONSE - len))>0) {
		len += n;
	}
	response[len]=0;

	printf("response:\n%s\n", response);

	close(sock);

	return 1;
}

	void
printUsage()
{
	printf("Usage: client host port\n");
	exit(1);
}

static void enter_callback( GtkWidget *widget,
		GtkWidget *entry )
{
	const gchar *entry_text;
	entry_text = gtk_entry_get_text (GTK_ENTRY (entry));
	printf ("Entry contents: %s\n", entry_text);
}

static void entry_toggle_editable( GtkWidget *checkbutton,
		GtkWidget *entry )
{
	gtk_editable_set_editable (GTK_EDITABLE (entry),
			GTK_TOGGLE_BUTTON (checkbutton)->active);
}

static void entry_toggle_visibility( GtkWidget *checkbutton,
		GtkWidget *entry )
{
	gtk_entry_set_visibility (GTK_ENTRY (entry),
			GTK_TOGGLE_BUTTON (checkbutton)->active);
}

static void list_clicked( GtkWidget * widget, gpointer data)
{
	//GtkTreeView * tree_view = (GtkTreeView*) data;
	//GtkTreeModel * theModel = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
	GtkTreeModel * model;
	GtkTreeIter iter;
	gchar * value = NULL;
	printf("Clicked rooms\n");
	if (gtk_tree_selection_get_selected( GTK_TREE_SELECTION(widget), &model, &iter)) {
		gtk_tree_model_get(model, &iter, 0, &value, -1);
		printf("Selected room is %s\n", value);
	}
	char * rroomm = strdup(currentRoom);
	if (value != NULL) {
		char v[100];
		strcpy(v,(char*)value);
		//g_free(value);
		strcpy(currentRoom,v);
	}
	char password[100];
	char response[MAX_RESPONSE];
	int i;
	for (i=0; i<n; i++) {
		if (!strcmp(userArray[i],currentUser)) {
			strcpy(password, passwordArray[i]);
		}
	}

	if (value != NULL && strcmp(value,rroomm)) {
	char msg3[200];
	strcpy(msg3,"SEND-MESSAGE ");
	strcat(msg3,currentUser);
	strcat(msg3," ");
	strcat(msg3,password);
	strcat(msg3," ");
	strcat(msg3,rroomm);
	strcat(msg3," leaves room");
	sendCommand(host, port, msg3, response);
	char msg0[200];
	strcpy(msg0,"LEAVE-ROOM ");
	strcat(msg0,currentUser);
	strcat(msg0," ");
	strcat(msg0,password);
	strcat(msg0," ");
	strcat(msg0,rroomm);
	sendCommand(host, port, msg0, response);
	char msg[200];
	strcpy(msg,"ENTER-ROOM ");
	strcat(msg,currentUser);
	strcat(msg," ");
	strcat(msg,password);
	strcat(msg," ");
	strcat(msg,currentRoom);
	sendCommand(host, port, msg, response);
	char msg2[200];
	strcpy(msg2,"SEND-MESSAGE ");
	strcat(msg2,currentUser);
	strcat(msg2," ");
	strcat(msg2,password);
	strcat(msg2," ");
	strcat(msg2,currentRoom);
	strcat(msg2," ");
	strcat(msg2,"entered room");
	sendCommand(host, port, msg2, response);
	}
}

static void user_clicked( GtkWidget * widget, gpointer data)
{
	GtkTreeSelection * selection = (GtkTreeSelection*) data;
	gtk_tree_selection_unselect_all(GTK_TREE_SELECTION(selection));
	strcpy(currentRoom, "DEFAULT");
	GtkTreeModel * model;
	GtkTreeIter iter;
	gchar * value = NULL;
	printf("Clicked users\n");
	if (gtk_tree_selection_get_selected( GTK_TREE_SELECTION(widget), &model, &iter)) {
		gtk_tree_model_get(model, &iter, 0, &value, -1);
		printf("Selected user is %s\n", value);
	}
	if (value != NULL) {
		char v[100];
		strcpy(v,(char*)value);
		//g_free(value);
		strcpy(currentUser,v);
	}
	int i;
	char password[100];
	char response[MAX_RESPONSE];
	for (i=0; i<n; i++) {
		if (!strcmp(userArray[i],currentUser)) {
			strcpy(password, passwordArray[i]);
		}
	}
}

static GtkWidget *create_list( char * text )
{

	GtkWidget *scrolled_window;
	GtkWidget *tree_view;
	GtkListStore *model;
	GtkTreeIter iter;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;

	int i;

	/* Create a new scrolled window, with scrollbars only if needed */
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
			GTK_POLICY_AUTOMATIC, 
			GTK_POLICY_AUTOMATIC);

	model = gtk_list_store_new (1, G_TYPE_STRING);

	tree_view = gtk_tree_view_new ();

	gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));
	gtk_widget_show (tree_view);

	/* Add some messages to the window */
	if (!strcmp(text,"User")) {
		char response[MAX_RESPONSE];
		sendCommand(host, port, "GET-USERS-IN-ROOM baker 323", response);
		char * token = strtok(response,"\r\n");
		gchar * msg;
		while (token != NULL) {
			msg = g_strdup_printf ("%s",token);
			gtk_list_store_append (GTK_LIST_STORE (model), &iter);
			gtk_list_store_set (GTK_LIST_STORE (model), 
					&iter,
					0, msg,
					-1);
			g_free (msg);
			token = strtok(NULL,"\r\n");
		}
	}
	else {
		char response[MAX_RESPONSE];
		sendCommand(host, port, "LIST-ROOMS baker 323", response);
		char * token = strtok(response,"\r\n");
		gchar * msg;
		while (token != NULL) {
			msg = g_strdup_printf ("%s",token);
			gtk_list_store_append (GTK_LIST_STORE (model), &iter);
			gtk_list_store_set (GTK_LIST_STORE (model), 
					&iter,
					0, msg,
					-1);
			g_free (msg);
			token = strtok(NULL,"\r\n");
		}
	}

	cell = gtk_cell_renderer_text_new ();

	if (!strcmp(text,"User")) {
		column = gtk_tree_view_column_new_with_attributes ("Users:",
				cell,
				"text", 0,
				NULL);
	}
	else {
		column = gtk_tree_view_column_new_with_attributes ("Rooms:",
				cell,
				"text", 0,
				NULL);
	}

	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
			GTK_TREE_VIEW_COLUMN (column));

	if (!strcmp(text,"User")) {
		GtkTreeModel * model2;
		GtkTreeSelection * selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
		gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE);
		g_signal_connect (selection, "changed", G_CALLBACK(user_clicked), NULL);
	}
	else {
		GtkTreeModel * model3;
		GtkTreeSelection * selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
		gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE);
		g_signal_connect (selection, "changed", G_CALLBACK(list_clicked), NULL);
	}

	return scrolled_window;
}

/* Add some text to our text widget - this is a callback that is invoked
   when our window is realized. We could also force our window to be
   realized with gtk_widget_realize, but it would have to be part of
   a hierarchy first */

static void insert_text( GtkTextBuffer *buffer, char * text )
{
	/*GtkTextIter iter;

	  gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);

	  gtk_text_buffer_insert (buffer, &iter,   
	  text, -1);
	  */
	gtk_text_buffer_set_text (buffer, text, -1);
}

/* Create a scrolled text area that displays a "message" */
static GtkWidget *create_text(char * text, GtkWidget * widget)
{
	GtkWidget *scrolled_window;
	GtkWidget *view;
	GtkTextBuffer *buffer;

	view = gtk_text_view_new ();
	gtk_text_view_set_editable ((GtkTextView*)view, FALSE);
	gtk_text_view_set_cursor_visible ((GtkTextView*)view, FALSE);
	widget = ((GtkWidget*)&view);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);

	gtk_container_add (GTK_CONTAINER (scrolled_window), view);
	insert_text (buffer,text);

	gtk_widget_show_all (scrolled_window);

	return scrolled_window;
}

static char buffer[256];

static gboolean time_handler(GtkWidget *widget)
{
	if (widget->window == NULL) return FALSE;
	/*
	   time_t curtime;
	   struct tm *loctime;

	   curtime = time(NULL);
	   loctime = localtime(&curtime);
	   strftime(buffer, 256, "%T", loctime);

	   gtk_widget_queue_draw(widget);
	   */
	if (strcmp(currentRoom,"DEFAULT")) {
		char response[MAX_RESPONSE];
		char password[100];
		int i;
		for (i=0; i<n; i++) {
			if (!strcmp(userArray[i],currentUser)) {
				strcpy(password, passwordArray[i]);
			}
		}

		/*	sendCommand(host, port, "GET-ALL-USERS baker 323", response);
			char * token = strtok(response,"\r\n");
			gchar * msg4;
			while (token != NULL) {
			msg4 = g_strdup_printf ("%s",token);
			gtk_list_store_append (GTK_LIST_STORE (model), &iter);
			gtk_list_store_set (GTK_LIST_STORE (model), 
			&iter,
			0, msg4,
			-1);
			g_free (msg4);
			token = strtok(NULL,"\r\n");
			}
			*/
		char msg0[200];
		strcpy(msg0, "ENTER-ROOM ");
		strcat(msg0, currentUser);
		strcat(msg0," ");
		strcat(msg0,password);
		strcat(msg0," ");
		strcat(msg0,currentRoom);
		sendCommand(host, port, msg0, response);

		char msg[200];
		strcpy(msg,"GET-MESSAGES ");
		strcat(msg,currentUser);
		strcat(msg," ");
		strcat(msg,password);
		strcat(msg," 0 ");
		strcat(msg,currentRoom);
		sendCommand(host, port, msg, response);
		GtkWidget *text;
		GtkTextBuffer * buffer;

		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW ((GtkTextView*)widget));
		GtkWidget * temp;
		text = create_text (response, temp);

		if (response[0] >= '0' && response[0] <= '9') {
			char * copy = strdup(response);
			char * token = strtok(copy,"\r\n");
			char bigmess[10000];
			int count = 0;
			while (token != NULL) {
				count++;
				char * tokcpy = strdup(token);
				char * end_str;
				char * token2 = strtok_r(tokcpy," ",&end_str);
				char line[200];
				int counter = 0;
				while (token2 != NULL) {
					counter++;
					if (counter==1) {}
					else if (counter==2) {
						strcpy(line,token2);
						strcat(line,":");
					}
					else {
						strcat(line," ");
						strcat(line,token2);
					}
					token2 = strtok_r(NULL," ",&end_str);
				}
				printf("%s\n", line);
				if (count==1) {
					strcpy(bigmess,line);
				}
				else {
					strcat(bigmess,"\r\n");
					strcat(bigmess,line);
				}
				token = strtok(NULL,"\r\n");
			}
			insert_text (buffer,bigmess);
		}
		else {
			insert_text (buffer, response);
		}

		gtk_widget_show (widget);
	}
	else {
		GtkTextBuffer * buffer;
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW ((GtkTextView*)widget));
		insert_text (buffer, "");
	}
	return TRUE;
}

static gboolean time_handler2(GtkWidget *widget)
{
	if (widget->window == NULL) return FALSE;
	/*
	   time_t curtime;
	   struct tm *loctime;

	   curtime = time(NULL);
	   loctime = localtime(&curtime);
	   strftime(buffer, 256, "%T", loctime);

	   gtk_widget_queue_draw(widget);
	   */
	if (strcmp(currentRoom,"DEFAULT")) {
		char response[MAX_RESPONSE];
		char password[100];
		int i;
		for (i=0; i<n; i++) {
			if (!strcmp(userArray[i],currentUser)) {
				strcpy(password, passwordArray[i]);
			}
		}

		/*	sendCommand(host, port, "GET-ALL-USERS baker 323", response);
			char * token = strtok(response,"\r\n");
			gchar * msg4;
			while (token != NULL) {
			msg4 = g_strdup_printf ("%s",token);
			gtk_list_store_append (GTK_LIST_STORE (model), &iter);
			gtk_list_store_set (GTK_LIST_STORE (model), 
			&iter,
			0, msg4,
			-1);
			g_free (msg4);
			token = strtok(NULL,"\r\n");
			}
			*/
		char msg0[200];
		strcpy(msg0, "ENTER-ROOM ");
		strcat(msg0, currentUser);
		strcat(msg0," ");
		strcat(msg0,password);
		strcat(msg0," ");
		strcat(msg0,currentRoom);
		sendCommand(host, port, msg0, response);

		char msg[200];
		strcpy(msg,"GET-USERS-IN-ROOM ");
		strcat(msg,currentUser);
		strcat(msg," ");
		strcat(msg,password);
		strcat(msg," ");
		strcat(msg,currentRoom);
		sendCommand(host, port, msg, response);
		GtkWidget *text;
		GtkTextBuffer * buffer;

		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW ((GtkTextView*)widget));
		GtkWidget * temp;
		text = create_text (response, temp);
		insert_text (buffer,response);
		gtk_widget_show (widget);
	}
	else {
		GtkTextBuffer * buffer;
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW ((GtkTextView*)widget));
		insert_text (buffer, "");
	}
	return TRUE;
}

static gboolean time_handler3( GtkListStore * widget) {
	GtkTreeModel * model = (GtkTreeModel*)widget;
	//if (widget->window == NULL) return FALSE;
	char response[MAX_RESPONSE];
	GtkTreeIter iter;
	sendCommand(host, port, "LIST-ROOMS baker 323", response);
	char * token = strtok(response,"\r\n");
	gchar * msg;
	gtk_list_store_clear (GTK_LIST_STORE (model));
	while (token != NULL) {
		msg = g_strdup_printf ("%s",token);
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), 
				&iter,
				0, msg,
				-1);
		g_free (msg);
		token = strtok(NULL,"\r\n");
	}
	printf("Updated rooms\n");
	return TRUE;
}

static gboolean time_handler4( GtkListStore * widget) {
	GtkTreeModel * model7 = (GtkTreeModel*)widget;
	char response[MAX_RESPONSE];
	GtkTreeIter iter7;
	sendCommand(host, port, "GET-ALL-USERS baker 323", response);
	char * token7 = strtok(response,"\r\n");
	gchar * msg7;
	gtk_list_store_clear (GTK_LIST_STORE (model7));
	while (token7 != NULL) {
		msg7 = g_strdup_printf ("%s",token7);
		gtk_list_store_append (GTK_LIST_STORE (model7), &iter7);
		gtk_list_store_set (GTK_LIST_STORE (model7), 
				&iter7,
				0, msg7,
				-1);
		g_free (msg7);
		token7 = strtok(NULL,"\r\n");
	}
	return TRUE;
}

static gboolean delete_event( GtkWidget *widget,
		GdkEvent  *event,
		gpointer   data )
{
	/* If you return FALSE in the "delete-event" signal handler,
	 * GTK will emit the "destroy" signal. Returning TRUE means
	 * you don't want the window to be destroyed.
	 * This is useful for popping up 'are you sure you want to quit?'
	 * type dialogs. */

	g_print ("delete event occurred\n");

	/* Change TRUE to FALSE and the main window will be destroyed with
	 * a "delete-event". */

	return TRUE;
}

/* Another callback */
static void destroy( GtkWidget *widget,
		gpointer   data )
{
	gtk_main_quit ();
}

static void check_password( GtkWidget ** object ) {

	GtkEntry * entry1 = (GtkEntry*)object[0];
	GtkEntry * entry2 = (GtkEntry*)object[1];
	GtkTreeModel * model = (GtkTreeModel*)object[2];
	GtkWidget * tree_view = (GtkWidget*)object[3];
	GtkWidget * scrolled_window = (GtkWidget*)object[4];

	const gchar * user = gtk_entry_get_text( GTK_ENTRY(entry1));
	const gchar * password = gtk_entry_get_text( GTK_ENTRY(entry2));

	printf("%s\n", user);
	printf("%s\n", password);

	strcpy(userArray[n], user);
	strcpy(passwordArray[n++],password);

	char * user1 = strdup(user);
	char * pswd1 = strdup(password);

	char msg[200];
	strcpy(msg, "ADD-USER ");
	strcat(msg, user1);
	strcat(msg, " ");
	strcat(msg, pswd1);

	printf("%s\n", msg);
	char response[MAX_RESPONSE];
	sendCommand(host, port, msg, response);
	printf("%s\n", response);
	if (!strcmp(response,"DENIED\r\n")) {
		GtkWidget * window;
		GtkWidget * button;
		GtkWidget *vbox;
		window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_container_set_border_width (GTK_CONTAINER (window), 10);

		vbox = gtk_vbox_new (FALSE, 0);
		gtk_container_add (GTK_CONTAINER (window), vbox);
		gtk_widget_show (vbox);

		GtkWidget * label;
		label = gtk_label_new ("Invalid Username or Password");
		gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
		gtk_widget_show (label);

		button = gtk_button_new_with_label ("OK");
		g_signal_connect (window, "delete-event",
				G_CALLBACK (delete_event), NULL);
		g_signal_connect (window, "destroy",
				G_CALLBACK (destroy), NULL);
		g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
		gtk_container_add (GTK_CONTAINER (vbox), button);
		gtk_widget_show (button);
		gtk_widget_show (window);
		gtk_main ();

	}
	else {
		GtkTreeIter iter;
		//gtk_list_store_clear (GTK_LIST_STORE (model));
		//sendCommand(host, port, "GET-ALL-USERS baker 323", response);
		
		gchar * msg4;
		msg4 = g_strdup_printf ("%s",user1);
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), 
					&iter,
					0, msg4,
					-1);
		g_free (msg4);
		/*char * token = strtok(response,"\r\n");
		gchar * msg4;
		while (token != NULL) {
			msg4 = g_strdup_printf ("%s",token);
			gtk_list_store_append (GTK_LIST_STORE (model), &iter);
			gtk_list_store_set (GTK_LIST_STORE (model), 
					&iter,
					0, msg4,
					-1);
			g_free (msg4);
			token = strtok(NULL,"\r\n");
			//gtk_widget_show (scrolled_window);
		}*/
	}
}

static void send_message ( GtkWidget * widget, gpointer data )
{
	GtkEntry * entry = (GtkEntry*) data;
	const gchar * message = gtk_entry_get_text( GTK_ENTRY(entry));
	char response[MAX_RESPONSE];
	char msg[200];
	char * msg1 = strdup(message);
	int i;
	char password[100];
	for (i=0; i<n; i++) {
		if (!strcmp(userArray[i],currentUser)) {
			strcpy(password, passwordArray[i]);
			printf("Password: %s\n", passwordArray[i]);
		}
	}
	char msg0[200];
	strcpy(msg0, "ENTER-ROOM ");
	strcat(msg0, currentUser);
	strcat(msg0," ");
	strcat(msg0,password);
	strcat(msg0," ");
	strcat(msg0,currentRoom);
	sendCommand(host, port, msg0, response);
	strcpy(msg, "SEND-MESSAGE ");
	strcat(msg,currentUser);
	strcat(msg," ");
	strcat(msg,password);
	strcat(msg," ");
	strcat(msg,currentRoom);
	strcat(msg, " ");
	strcat(msg, msg1);
	printf("%s\n", msg);
	sendCommand(host, port, msg, response);
	gtk_entry_set_text(GTK_ENTRY(entry), "");
}

static void new_room( GtkWidget ** object) {
	GtkEntry * entry1 = (GtkEntry*)object[0];
	const gchar * message = gtk_entry_get_text( GTK_ENTRY(entry1));
	GtkTreeModel * model = (GtkTreeModel*)object[1];
	GtkWidget * tree_view = (GtkWidget*)object[2];

	GtkTreeIter iter;
	char response[MAX_RESPONSE];
	gtk_list_store_clear (GTK_LIST_STORE (model));
	printf("%s\n", message);
	char msgg[100];
	strcpy(msgg,"CREATE-ROOM baker 323 ");
	strcat(msgg,message);
	sendCommand(host, port, msgg, response);
	sendCommand(host, port, "LIST-ROOMS baker 323", response);
	char * token = strtok(response,"\r\n");
	gchar * msg;
	while (token != NULL) {
		msg = g_strdup_printf ("%s",token);
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), 
				&iter,
				0, msg,
				-1);
		g_free (msg);
		token = strtok(NULL,"\r\n");
	}

}

static void double_click( GtkWidget * widget, gpointer data) {
	printf("Add room pressed!\n");
	//GtkListStore * model = (GtkListStore*)widget;
	GtkWidget * tree_view = (GtkWidget*) data;
	GtkTreeModel * model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
	GtkWidget * window;
	GtkWidget * button;
	GtkWidget *entry, *entry2;
	GtkWidget *vbox, *hbox, *hbox2;
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);

	GtkWidget * label;
	label = gtk_label_new ("Enter room name:");
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
	gtk_widget_show (label);

	entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (entry), 50);
	gtk_box_pack_start (GTK_BOX (vbox), entry, TRUE, TRUE, 0);
	gtk_widget_show (entry);

	button = gtk_button_new_with_label ("OK");
	g_signal_connect (window, "delete-event",
			G_CALLBACK (delete_event), NULL);
	g_signal_connect (window, "destroy",
			G_CALLBACK (destroy), NULL);

	GtkWidget ** object2;
	object2 = g_new(GtkWidget*, 3);
	object2[0] = entry;
	object2[1] = (GtkWidget*)model;
	object2[2] = tree_view;

	printf("Going to new room screen\n");
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (new_room), object2); 
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
	gtk_container_add (GTK_CONTAINER (vbox), button);
	gtk_widget_show (button);
	gtk_widget_show (window);
	gtk_main ();

}

static void create_user( GtkWidget *widget,
		GtkWidget ** object)
{

	GtkWidget * scrolled_window = (GtkWidget*)object[0];
	GtkWidget * tree_view = (GtkWidget*)object[1];
	GtkTreeModel * model = (GtkTreeModel*)object[2];

	GtkWidget * window;
	GtkWidget * button;
	GtkWidget *entry, *entry2;
	GtkWidget *vbox, *hbox, *hbox2;
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (vbox), hbox);
	gtk_widget_show (hbox);

	GtkWidget * label;
	label = gtk_label_new ("Username");
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
	gtk_widget_show (label);

	entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (entry), 50);
	gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
	gtk_widget_show (entry);

	hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (vbox), hbox2);
	gtk_widget_show (hbox2);

	GtkWidget * label2;
	label2 = gtk_label_new ("Password");
	gtk_box_pack_start (GTK_BOX (hbox2), label2, TRUE, TRUE, 0);
	gtk_widget_show (label2);

	entry2 = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (entry2), 50);
	gtk_entry_set_visibility (GTK_ENTRY (entry2),FALSE);
	gtk_box_pack_start (GTK_BOX (hbox2), entry2, TRUE, TRUE, 0);
	gtk_widget_show (entry2);

	button = gtk_button_new_with_label ("OK");
	g_signal_connect (window, "delete-event",
			G_CALLBACK (delete_event), NULL);
	g_signal_connect (window, "destroy",
			G_CALLBACK (destroy), NULL);

	GtkWidget ** object2;
	object2 = g_new(GtkWidget*, 5);
	object2[0] = entry;
	object2[1] = entry2;
	object2[2] = (GtkWidget*)model;
	object2[3] = tree_view;
	object2[4] = scrolled_window;

	g_signal_connect_swapped (button, "clicked", G_CALLBACK (check_password), object2); 
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
	gtk_container_add (GTK_CONTAINER (vbox), button);
	gtk_widget_show (button);
	gtk_widget_show (window);
	gtk_main ();
	g_print ("Hello World\n");
	return;
}

int
main(int argc, char **argv) {

	//char * command;

	if (argc < 3) {
		printUsage();
	}

	host = argv[1];
	sport = argv[2];
	//command = argv[3];

	sscanf(sport, "%d", &port);

	char response[MAX_RESPONSE];
	sendCommand(host, port, "ADD-USER baker 323", response);
	sendCommand(host, port, "GET-ALL-USERS baker 323", response);

	GtkWidget *window;
	GtkWidget *vbox, *hbox, *hbox2;
	GtkWidget *entry;
	GtkWidget *button, *button2;
	GtkWidget *check;
	gint tmp_pos;

	//GtkWidget *window;
	GtkWidget *vpaned;
	GtkWidget *list;
	GtkWidget *list2;
	GtkWidget *list3;
	GtkWidget *text;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "IRC Client");
	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_widget_set_size_request (GTK_WIDGET (window), 500, 500);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show (vbox);
	/* create a vpaned widget and add it to our toplevel window */

	GtkWidget * label0;
	label0 = gtk_label_new ("Welcome to the Chat client!");
	gtk_widget_modify_font (label0, pango_font_description_from_string("Tahoma 16"));
	gtk_box_pack_start (GTK_BOX (vbox), label0, TRUE, TRUE, 0);
	gtk_widget_show (label0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_set_spacing(GTK_BOX(hbox), 2);
	gtk_container_add (GTK_CONTAINER (vbox), hbox);
	gtk_widget_show (hbox);

	/*	vpaned = gtk_vpaned_new ();
		gtk_container_add (GTK_CONTAINER (window), vpaned);
		gtk_widget_show (vpaned);
		*/
	/* Now create the contents of the two halves of the window */

	//list = create_list ("Room");
	/*	sendCommand(host, port, "GET-ALL-USERS baker 323", response);
		char * token = strtok(response,"\r\n");
		gchar * msg4;
		while (token != NULL) {
		msg4 = g_strdup_printf ("%s",token);
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), 
		&iter,
		0, msg4,
		-1);
		g_free (msg4);
		token = strtok(NULL,"\r\n");
		}
		*/
	strcpy(userArray[n], "baker");
	strcpy(passwordArray[n++],"323");
	sendCommand(host, port, "ADD-USER person 123", response);
	strcpy(userArray[n], "person");
	strcpy(passwordArray[n++],"123");

	sendCommand(host, port, "CREATE-ROOM person 123 room1", response);
	//sendCommand(host, port, "ENTER-ROOM baker 323 room1", response);
	sendCommand(host, port, "CREATE-ROOM person 123 room2", response);
	sendCommand(host, port, "CREATE-ROOM person 123 room3", response);
	sendCommand(host, port, "CREATE-ROOM person 123 room4", response);
	sendCommand(host, port, "CREATE-ROOM person 123 room5", response);
	sendCommand(host, port, "CREATE-ROOM person 123 room6", response);
	sendCommand(host, port, "CREATE-ROOM person 123 room7", response);
	sendCommand(host, port, "CREATE-ROOM person 123 room8", response);
	sendCommand(host, port, "CREATE-ROOM person 123 room9", response);
	sendCommand(host, port, "LIST-ROOMS person 123", response);

	//list2 = create_list ("User");
	GtkWidget *scrolled_window7;
	GtkWidget *tree_view7;
	GtkListStore *model7;
	GtkTreeIter iter7;
	GtkCellRenderer *cell7;
	GtkTreeViewColumn *column7;

	/* Create a new scrolled window, with scrollbars only if needed */
	scrolled_window7 = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window7),
			GTK_POLICY_AUTOMATIC, 
			GTK_POLICY_AUTOMATIC);

	model7 = gtk_list_store_new (1, G_TYPE_STRING);

	tree_view7 = gtk_tree_view_new ();

	gtk_container_add (GTK_CONTAINER (scrolled_window7), tree_view7);
	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view7), GTK_TREE_MODEL (model7));
	gtk_widget_show (tree_view7);

	/* Add some messages to the window */
	sendCommand(host, port, "GET-ALL-USERS baker 323", response);
	char * token7 = strtok(response,"\r\n");
	gchar * msg7;
	while (token7 != NULL) {
		msg7 = g_strdup_printf ("%s",token7);
		gtk_list_store_append (GTK_LIST_STORE (model7), &iter7);
		gtk_list_store_set (GTK_LIST_STORE (model7), 
				&iter7,
				0, msg7,
				-1);
		g_free (msg7);
		token7 = strtok(NULL,"\r\n");
	}

	//g_timeout_add(5000, (GSourceFunc) time_handler4, (gpointer) model7);
	//time_handler4(model7);

	cell7 = gtk_cell_renderer_text_new ();

	column7 = gtk_tree_view_column_new_with_attributes ("Users:",
			cell7,
			"text", 0,
			NULL);

	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view7),
			GTK_TREE_VIEW_COLUMN (column7));

	GtkTreeModel * model2;
	GtkTreeSelection * selection7 = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view7));
	gtk_tree_selection_set_mode( selection7, GTK_SELECTION_BROWSE);
	//g_signal_connect (selection7, "changed", G_CALLBACK(user_clicked), tree_view7);

	gtk_box_pack_start (GTK_BOX (hbox), scrolled_window7, TRUE, TRUE, 0);
	gtk_widget_show (scrolled_window7);

	GtkWidget *scrolled_window2;
	GtkWidget *tree_view;
	GtkListStore *model;
	GtkTreeIter iter;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;

	int i;

	/* Create a new scrolled window, with scrollbars only if needed */
	scrolled_window2 = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window2),
			GTK_POLICY_AUTOMATIC, 
			GTK_POLICY_AUTOMATIC);

	model = gtk_list_store_new (1, G_TYPE_STRING);
	
	tree_view = gtk_tree_view_new ();

	// create new room when room list is double clicked
	
	gtk_container_add (GTK_CONTAINER (scrolled_window2), tree_view);
	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));
	gtk_widget_show (tree_view);

	/* Add some messages to the window */
	sendCommand(host, port, "LIST-ROOMS baker 323", response);
	char * token = strtok(response,"\r\n");
	gchar * msg;
	while (token != NULL) {
		msg = g_strdup_printf ("%s",token);
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), 
				&iter,
				0, msg,
				-1);
		g_free (msg);
		token = strtok(NULL,"\r\n");
	}

	g_timeout_add(5000, (GSourceFunc) time_handler3, (gpointer) model);
	time_handler3(model);

	cell = gtk_cell_renderer_text_new ();

	column = gtk_tree_view_column_new_with_attributes ("Rooms:",
			cell,
			"text", 0,
			NULL);
	gtk_tree_view_column_set_clickable(GTK_TREE_VIEW_COLUMN(column), TRUE);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN(column), -1);
	g_signal_connect(column, "clicked", G_CALLBACK(double_click), tree_view);

	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
			GTK_TREE_VIEW_COLUMN (column));

	GtkTreeModel * model3;
	GtkTreeSelection * selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE);
	g_signal_connect (selection, "changed", G_CALLBACK(list_clicked), NULL);
	g_signal_connect (selection7, "changed", G_CALLBACK(user_clicked), selection);
	gtk_box_pack_start (GTK_BOX (hbox), scrolled_window2, TRUE, TRUE, 0);
	gtk_widget_show (scrolled_window2);

	GtkWidget * label;
	label = gtk_label_new ("Messages:");
	gtk_widget_modify_font (label, pango_font_description_from_string("Tahoma 12"));
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
	gtk_widget_show (label);

	//text = create_text (response, widget);

	GtkWidget *scrolled_window;
	GtkWidget *view;
	GtkTextBuffer *buffer;

	view = gtk_text_view_new ();
	gtk_text_view_set_editable ((GtkTextView*)view, FALSE);
	gtk_text_view_set_cursor_visible ((GtkTextView*)view, FALSE);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);

	gtk_container_add (GTK_CONTAINER (scrolled_window), view);
	//insert_text (buffer,response);

	gtk_widget_show_all (scrolled_window);
	gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);
	gtk_widget_show (scrolled_window);

	g_timeout_add(5000, (GSourceFunc) time_handler, (gpointer) view);
	gtk_widget_show_all(view);
	time_handler(view);

	GtkWidget * scrolled_windoww;
	GtkWidget * labell;
	labell = gtk_label_new ("Users in room:");
	gtk_widget_modify_font (labell, pango_font_description_from_string("Tahoma 12"));
	gtk_box_pack_start (GTK_BOX (vbox), labell, TRUE, TRUE, 0);
	gtk_widget_show (labell);

	GtkWidget * vieww;
	GtkTextBuffer * bufferr;

	vieww = gtk_text_view_new ();
	gtk_text_view_set_editable ((GtkTextView*)vieww, FALSE);
	gtk_text_view_set_cursor_visible ((GtkTextView*)vieww, FALSE);
	bufferr = gtk_text_view_get_buffer (GTK_TEXT_VIEW (vieww));

	scrolled_windoww = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_windoww),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);

	gtk_container_add (GTK_CONTAINER (scrolled_windoww), vieww);
	//insert_text (buffer,response);

	gtk_widget_show_all (scrolled_windoww);
	gtk_box_pack_start (GTK_BOX (vbox), scrolled_windoww, TRUE, TRUE, 0);
	gtk_widget_show (scrolled_windoww);

	g_timeout_add(5000, (GSourceFunc) time_handler2, (gpointer) vieww);
	gtk_widget_show_all(vieww);
	time_handler(vieww);

	GtkWidget * label2;
	label2 = gtk_label_new ("Type a message:");
	gtk_widget_modify_font (label2, pango_font_description_from_string("Tahoma 12"));
	gtk_box_pack_start (GTK_BOX (vbox), label2, TRUE, TRUE, 0);
	gtk_widget_show (label2);

	entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (entry), 50);
	g_signal_connect (entry, "activate",
			G_CALLBACK (send_message),
			entry);
	//gtk_entry_set_text (GTK_ENTRY (entry), "hello");
	//tmp_pos = GTK_ENTRY (entry)->text_length;
	//gtk_editable_insert_text (GTK_EDITABLE (entry), " world", -1, &tmp_pos);
	gtk_editable_select_region (GTK_EDITABLE (entry),
			0, GTK_ENTRY (entry)->text_length);
	gtk_box_pack_start (GTK_BOX (vbox), entry, TRUE, TRUE, 0);
	gtk_widget_show (entry);

	hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (vbox), hbox2);
	gtk_widget_show (hbox2);

	GtkWidget * button0;
	button0 = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_button_set_label ((GtkButton*) button0, "Send");

	g_signal_connect (button0, "clicked",
			G_CALLBACK (send_message),
			entry);
	gtk_box_pack_start (GTK_BOX (hbox2), button0, TRUE, TRUE, 0);
	gtk_widget_set_can_default (button0, TRUE);
	gtk_widget_grab_default (button0);
	gtk_widget_show (button0);

	button2 = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_button_set_label ((GtkButton*) button2, "Create Account");

	GtkTreeModel *modelo = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	GtkWidget ** object;
	object = g_new(GtkWidget*, 3);
	object[0] = (GtkWidget*)scrolled_window7;
	object[1] = (GtkWidget*)tree_view7;
	object[2] = (GtkWidget*)model7;

	g_signal_connect (button2, "clicked",
			G_CALLBACK (create_user),
			object);
	gtk_box_pack_start (GTK_BOX (hbox2), button2, TRUE, TRUE, 0);
	gtk_widget_set_can_default (button2, TRUE);
	gtk_widget_grab_default (button2);
	gtk_widget_show (button2);
	/* Creates a new button with the label "Hello World". */

	gtk_widget_show (window);

	gtk_main ();

	return 0;
}
