document.addEventListener('DOMContentLoaded', () => {
    const chatWindow = document.getElementById('chat-window');
    const userInput = document.getElementById('user-input');
    const sendBtn = document.getElementById('send-btn');
    const helpPanel = document.getElementById('help-panel');
    const toggleHelpBtn = document.getElementById('toggle-help');
    const closeHelpBtn = document.getElementById('close-help');

    function addMessage(text, sender) {
        const msg = document.createElement('div');
        msg.classList.add('message', sender);
        msg.innerHTML = `<div class="message-content">${text}</div>`;
        chatWindow.appendChild(msg);
        chatWindow.scrollTop = chatWindow.scrollHeight;
    }

    function mockSystemResponse(userText) {
        const lower = userText.toLowerCase();
        let reply = "Hey!";
        
        if (lower.includes('привет') || lower.includes('здравствуй')) {
            reply = "Здравствуйте! Чем могу помочь по теме досуга?";
        } else if (lower.includes('фильм') || lower.includes('кино')) {
            reply = "В области досуга популярны следующие жанры: приключения, комедии, документальное кино. Уточните, что вас интересует?";
        } else if (lower.includes('книг') || lower.includes('читать')) {
            reply = "Чтение — отличный вариант досуга! Могу порекомендовать жанры: детективы, фантастика, нон-фикшн.";
        } else if (lower.includes('путешеств') || lower.includes('туризм')) {
            reply = "Путешествия — замечательный способ провести свободное время. Интересует внутренний или зарубежный туризм?";
        }

        setTimeout(() => addMessage(reply, 'system'), 500 + Math.random() * 400);
    }

    function handleSend() {
        const text = userInput.value.trim();
        if (!text) return;
        
        addMessage(text, 'user');
        userInput.value = '';
        userInput.style.height = 'auto'; 
        mockSystemResponse(text);
    }

    sendBtn.addEventListener('click', handleSend);
    
    userInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            handleSend();
        }
    });

    userInput.addEventListener('input', function() {
        this.style.height = 'auto';
        if (this.scrollHeight <= 120) {
            this.style.height = this.scrollHeight + 'px';
        } else {
            this.style.height = '120px'; 
        }
    });

    toggleHelpBtn.addEventListener('click', (e) => {
        e.stopPropagation();
        helpPanel.classList.add('open');
    });
    
    closeHelpBtn.addEventListener('click', () => {
        helpPanel.classList.remove('open');
    });
 
    document.addEventListener('click', (e) => {
        if (helpPanel.classList.contains('open') && 
            !helpPanel.contains(e.target) && 
            !toggleHelpBtn.contains(e.target)) {
            helpPanel.classList.remove('open');
        }
    });
});