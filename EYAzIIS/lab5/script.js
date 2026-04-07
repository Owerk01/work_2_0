document.addEventListener('DOMContentLoaded', () => {
    const chatWindow = document.getElementById('chat-window');
    const userInput = document.getElementById('user-input');
    const sendBtn = document.getElementById('send-btn');
    const deleteBtn = document.getElementById('delete-chat-btn');
    const helpPanel = document.getElementById('help-panel');
    const toggleHelpBtn = document.getElementById('toggle-help');
    const closeHelpBtn = document.getElementById('close-help');
    const currentUserEl = document.getElementById('current-user');

    let USER_ID = localStorage.getItem('hobby_user_id');
    if (!USER_ID) {
        USER_ID = Math.floor(Math.random() * 1e9).toString();
        localStorage.setItem('hobby_user_id', USER_ID);
    }
    if (currentUserEl) currentUserEl.textContent = `User: ${USER_ID}`;

    function escapeHtml(str) {
        const div = document.createElement('div');
        div.textContent = str;
        return div.innerHTML;
    }

    function scrollToBottom() {
        chatWindow.scrollTop = chatWindow.scrollHeight;
    }

    function addMessage(text, sender, position) {
        const msg = document.createElement('div');
        msg.classList.add('message', sender);
        if (position !== undefined && position !== null) {
            msg.dataset.position = position;
        }
        msg.innerHTML = `
            <div class="message-content">${escapeHtml(text)}</div>
            <div class="msg-actions" style="display: ${position ? '' : 'none'};">
                <button class="edit-btn" title="Edit">✏️</button>
                <button class="delete-btn" title="Delete">🗑️</button>
            </div>
        `;
        chatWindow.appendChild(msg);
        scrollToBottom();
    }

    chatWindow.addEventListener('click', async (e) => {
        const btn = e.target.closest('.edit-btn, .delete-btn');
        if (!btn) return;

        const msgEl = btn.closest('.message');
        const position = msgEl.dataset.position;
        const sender = msgEl.classList.contains('user') ? 'user' : 'system';
        const contentEl = msgEl.querySelector('.message-content');

        if (btn.classList.contains('delete-btn')) {
            if (!confirm('Удалить этот ход разговора (вопрос + ответ)?')) return;
            msgEl.classList.add('deleting');
            try {
                await fetch(`/api/chat/${USER_ID}/message/${position}`, { method: 'DELETE' });
                setTimeout(() => msgEl.remove(), 300);
            } catch (err) {
                msgEl.classList.remove('deleting');
                alert('Ошибка удаления: ' + err.message);
            }
        } else if (btn.classList.contains('edit-btn')) {
            const currentText = contentEl.textContent;
            contentEl.innerHTML = `
                <textarea class="edit-area">${escapeHtml(currentText)}</textarea>
                <div class="edit-actions">
                    <button class="save-btn">💾</button>
                    <button class="cancel-btn">❌</button>
                </div>`;
            btn.parentElement.style.display = 'none';

            const textarea = contentEl.querySelector('.edit-area');
            textarea.focus();
            textarea.style.height = textarea.scrollHeight + 'px';

            const saveEdit = async () => {
                const newText = textarea.value.trim();
                if (!newText || newText === currentText) {
                    contentEl.textContent = currentText;
                    msgEl.querySelector('.msg-actions').style.display = '';
                    return;
                }
                try {
                    await fetch(`/api/chat/${USER_ID}/message/${position}`, {
                        method: 'PATCH',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({ role: sender, text: newText })
                    });
                    contentEl.textContent = newText;
                    contentEl.innerHTML = escapeHtml(newText);
                    msgEl.querySelector('.msg-actions').style.display = '';
                } catch (err) {
                    alert('Ошибка сохранения: ' + err.message);
                }
            };

            contentEl.querySelector('.save-btn').onclick = saveEdit;
            contentEl.querySelector('.cancel-btn').onclick = () => {
                contentEl.textContent = currentText;
                msgEl.querySelector('.msg-actions').style.display = '';
            };
            textarea.addEventListener('keydown', (ev) => {
                if (ev.key === 'Enter' && !ev.shiftKey) { ev.preventDefault(); saveEdit(); }
                if (ev.key === 'Escape') contentEl.querySelector('.cancel-btn').click();
            });
        }
    });

    async function loadChat() {
        try {
            const res = await fetch(`/api/chat/${USER_ID}`);
            if (res.ok) {
                const messages = await res.json();
                chatWindow.innerHTML = '';
                messages.forEach(msg => addMessage(msg.text, msg.sender, msg.position));
            }
        } catch (e) {
            console.warn('Could not load chat:', e);
        }
    }

    async function handleSend() {
        const text = userInput.value.trim();
        if (!text) return;
        userInput.value = '';
        userInput.style.height = 'auto';

        const userMsg = document.createElement('div');
        userMsg.classList.add('message', 'user');
        userMsg.innerHTML = `
            <div class="message-content">${escapeHtml(text)}</div>
            <div class="msg-actions" style="display: none;">
                <button class="edit-btn" title="Edit">✏️</button>
                <button class="delete-btn" title="Delete">🗑️</button>
            </div>`;
        chatWindow.appendChild(userMsg);
        scrollToBottom();

        const typing = document.createElement('div');
        typing.className = 'message system';
        typing.innerHTML = `<div class="message-content">Thinking...</div>`;
        chatWindow.appendChild(typing);
        scrollToBottom();

        try {
            const res = await fetch('/api/chat', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ user_id: USER_ID, query: text })
            });
            typing.remove();

            if (!res.ok) {
                const err = await res.json().catch(() => ({}));
                throw new Error(err.error || 'API error');
            }

            const data = await res.json();
            addMessage(data.response, 'system', data.position);

            userMsg.dataset.position = data.position;
            userMsg.querySelector('.msg-actions').style.display = '';
        } catch (error) {
            typing.remove();
            addMessage(`Error: ${error.message}`, 'system');
            console.error('Chat error:', error);
        }
    }

    async function handleDeleteChat() {
        if (!confirm('Delete entire chat history?')) return;
        try {
            const res = await fetch(`/api/chat/${USER_ID}`, { method: 'DELETE' });
            if (res.ok) chatWindow.innerHTML = '';
        } catch (e) {
            alert('Failed to delete: ' + e.message);
        }
    }

    sendBtn.addEventListener('click', handleSend);
    if (deleteBtn) deleteBtn.addEventListener('click', handleDeleteChat);
    userInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' && !e.shiftKey) { e.preventDefault(); handleSend(); }
    });
    userInput.addEventListener('input', function() {
        this.style.height = 'auto';
        if (this.scrollHeight <= 120) this.style.height = this.scrollHeight + 'px';
        else this.style.height = '120px';
    });
    if (toggleHelpBtn) {
        toggleHelpBtn.addEventListener('click', (e) => {
            e.stopPropagation();
            helpPanel.classList.add('open');
        });
    }
    if (closeHelpBtn) {
        closeHelpBtn.addEventListener('click', () => helpPanel.classList.remove('open'));
    }
    loadChat();
});