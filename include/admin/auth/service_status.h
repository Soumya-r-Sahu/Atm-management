#ifndef SERVICE_STATUS_H
#define SERVICE_STATUS_H

/**
 * Get current service status
 * 
 * @return 0 if ATM is online, 1 if offline/out of service
 */
int getServiceStatus(void);

/**
 * Set service status
 * 
 * @param isOutOfService 0 for Online, 1 for Offline
 * @return 1 if successful, 0 otherwise
 */
int setServiceStatus(int isOutOfService);

/**
 * Toggle the ATM service mode between Online and Offline
 * 
 * @return 1 if successful, 0 otherwise
 */
int toggleServiceMode(void);

#endif /* SERVICE_STATUS_H */