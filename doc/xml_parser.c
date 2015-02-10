
int handle_xml_query (usbphone* p, msg_t* msg) {
	// FIXME: da riscrivere
    /*
	int kmsent = 0;
	char contacts [][20] = {"Franz", "Berta", "Marty", "Giacomo"};
	int contacts_num = 4;
	char buffer[MAX_BUF_LEN];

	// FIXME: gestire il caso <cl>x</cl> -> MENU
	// FIXME: individuare il significato di <km> (se lo mando con ogni riga scrolla automaticamente e usa una sola riga)
	// FIXME: gestire il caso <cf i="x">y</cf> -> CHIAMATA, x = attributo "s", y = attributo "i" del tag <f> corrispondente
	// l'attributo "i" ed "s" negli elementi <f>  formano un id ?
	if (!kmsent++) {
		buffer[0] = p->session_id;
		snprintf(buffer+1, MAX_BUF_LEN-1, "<km>%d</km>", 1);
		sendMessage(phone, 0x59, strlen(buffer+1)+1, buffer); 	
	}

	int efflen = min(MAX_BUF_LEN-1, msg->len -1);
	memcpy(buffer, msg->data+1, efflen);
	buffer[efflen+1] = 0x00;
	char* i = strstr(buffer, "<rl>");
	char* f = strstr(buffer, "</rl>");
	int index = -1;
	if (i == buffer && f > i) {
		*f = 0x00;
		index = atoi(i+4);
		
		if (index < 0 || index > contacts_num) {
			LOG(ERROR, "Invalid 0x59 request (index=%d): %s\n", index, buffer);
			send_hangup(phone);
			return -1;
		}
		if (index == 0) index = contacts_num;
		buffer[0] = p->session_id;
		snprintf(buffer+1, MAX_BUF_LEN-1, "<l %s=\"%d\"><f i=\"%d\" s=\"%d\">%s</f></l>", (index == contacts_num) ? "e":"i", index, index+20, index +10, contacts[index-1]);
		sendMessage(phone, 0x59, strlen(buffer+1)+1, buffer); 	
		LOG(DEBUG, ">>> %s\n", buffer);
		return 0;

	} else {
		LOG(ERROR, "Invalid 0x59 request (%x, %x, %x): %s\n", i,f,buffer,buffer);
		send_hangup(phone);
		return -1;
	}
    */
	return send_hangup(p);
}


