USE core_banking_system;

DELIMITER //
CREATE PROCEDURE check_daily_withdrawal_limit(
    IN p_card_number VARCHAR(20),
    IN p_amount DECIMAL(10,2),
    IN p_channel ENUM('ATM', 'POS', 'ONLINE') DEFAULT 'ATM',
    OUT p_can_withdraw BOOLEAN,
    OUT p_remaining_limit DECIMAL(10,2),
    OUT p_message VARCHAR(255)
)
BEGIN
    DECLARE v_total_today DECIMAL(10,2);
    DECLARE v_daily_limit DECIMAL(10,2);
    DECLARE v_today DATE;
    DECLARE v_card_status VARCHAR(20);
    DECLARE v_account_status VARCHAR(20);
    DECLARE v_account_number VARCHAR(20);
    
    SET v_today = CURDATE();
    
    -- Check card status
    SELECT c.status, c.account_id, a.status 
    INTO v_card_status, v_account_number, v_account_status
    FROM cbs_cards c
    JOIN cbs_accounts a ON c.account_id = a.account_number
    WHERE c.card_number = p_card_number;
    
    -- Validate card and account status
    IF v_card_status IS NULL THEN
        SET p_can_withdraw = FALSE;
        SET p_remaining_limit = 0;
        SET p_message = 'Invalid card number';
        RETURN;
    ELSEIF v_card_status != 'ACTIVE' THEN
        SET p_can_withdraw = FALSE;
        SET p_remaining_limit = 0;
        SET p_message = CONCAT('Card is ', v_card_status);
        RETURN;
    ELSEIF v_account_status != 'ACTIVE' THEN
        SET p_can_withdraw = FALSE;
        SET p_remaining_limit = 0;
        SET p_message = CONCAT('Account is ', v_account_status);
        RETURN;
    END IF;
    
    -- Get card's daily limit based on channel
    IF p_channel = 'ATM' THEN
        SELECT daily_atm_limit INTO v_daily_limit
        FROM cbs_cards
        WHERE card_number = p_card_number;
    ELSEIF p_channel = 'POS' THEN
        SELECT daily_pos_limit INTO v_daily_limit
        FROM cbs_cards
        WHERE card_number = p_card_number;
    ELSE
        SELECT daily_online_limit INTO v_daily_limit
        FROM cbs_cards
        WHERE card_number = p_card_number;
    END IF;
    
    -- Get total withdrawals today
    SELECT COALESCE(SUM(amount), 0) INTO v_total_today
    FROM cbs_daily_withdrawals
    WHERE card_number = p_card_number 
    AND withdrawal_date = v_today;
    
    -- Calculate remaining limit
    SET p_remaining_limit = v_daily_limit - v_total_today;
    
    -- Check if withdrawal is allowed
    IF (v_total_today + p_amount) <= v_daily_limit THEN
        SET p_can_withdraw = TRUE;
        SET p_message = 'Withdrawal allowed';
    ELSE
        SET p_can_withdraw = FALSE;
        SET p_message = CONCAT('Exceeds daily limit. Remaining limit: ', p_remaining_limit);
    END IF;
END//
DELIMITER ;
