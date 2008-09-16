/* mmc.h - header file for mmap cache package
 */

#ifndef _MMC_H_
#define _MMC_H_

/* Returns an mmap()ed area for the given file, or (void*) 0 on errors.
** If you have a stat buffer on the file, pass it in, otherwise pass 0.
** Same for the current time.
*/
extern void* mmc_map( char* filename, struct stat* sbP, struct timeval* nowP );

/* Done with an mmap()ed area that was returned by mmc_map().
** If you have a stat buffer on the file, pass it in, otherwise pass 0.
** Same for the current time.
*/
extern void mmc_unmap( void* addr, struct stat* sbP, struct timeval* nowP );

/* Clean up the mmc package, freeing any unused storage.
** This should be called periodically, say every five minutes.
** If you have the current time, pass it in, otherwise pass 0.
*/
extern void mmc_cleanup( struct timeval* nowP );

/* Free all storage, usually in preparation for exitting. */
extern void mmc_destroy( void );

/* Generate debugging statistics syslog message. */
extern void mmc_logstats( long secs );

#endif /* _MMC_H_ */
