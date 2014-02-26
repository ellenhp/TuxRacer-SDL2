package com.moonlite.tuxracer;

import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.ArrayList;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.EditText;

// Scoreloop interface
import com.scoreloop.client.android.core.controller.RequestController;
import com.scoreloop.client.android.core.controller.RequestControllerObserver;
import com.scoreloop.client.android.core.controller.ScoreController;
import com.scoreloop.client.android.core.controller.ScoresController;
import com.scoreloop.client.android.core.controller.TermsOfServiceController;
import com.scoreloop.client.android.core.controller.TermsOfServiceControllerObserver;
import com.scoreloop.client.android.core.controller.UserController;
import com.scoreloop.client.android.core.model.Client;
import com.scoreloop.client.android.core.model.Score;
import com.scoreloop.client.android.core.model.Session;

import static tv.ouya.console.api.OuyaController.BUTTON_O;

import org.libsdl.app.SDLActivity;

/**
 * GameActivity implements (both Amazon and OUYA) In-App Purchase on top of SDLActivity
 */
public class ScoreActivity extends SDLActivity {

	private static final String TAG = "Tuxracer";

	private static boolean mUserCanSubmitScores = false;
	private static Runnable mScoreToSubmit = null;

	private static String mScoreloopUsername = "";
	
	// Load the .so
	static {
		System.loadLibrary("SDL2");
		System.loadLibrary("SDL2_image");
		System.loadLibrary("SDL2_mixer");
		// System.loadLibrary("SDL2_net");
		// System.loadLibrary("SDL2_ttf");
		System.loadLibrary("tcl");
		// Step 1 Load libscoreloopcore.so before loading your game's native
		// library.
		// System.loadLibrary("scoreloopcore");
		System.loadLibrary("main");
	}

	private String xor_secret(String secret) {
		int i;
		char[] buf = new char[secret.length()];
		char[] secretBuf = secret.toCharArray();
		int len = secret.length();
		for (i = 0; i < len; ++i) {
			buf[i] = (char) (secretBuf[i] ^ (i & 15) + 1);
		}
		return new String(buf);
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		if (keyCode == BUTTON_O) {
			View focusedButton = getCurrentFocus();
			focusedButton.performClick();
			return true;
		}
		return super.onKeyUp(keyCode, event);
	}

	public static native void nativeScoreloopGotScores(int scoreMode, Object[] scoreStrings);
	public static native void nativeTextCallback(String string);
	public static native void nativeDisableAliasPrompt();
	public static native void nativeUpdateUserInfo(String alias);
	
	// Setup
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// Log.v("SDL", "onCreate()");
		super.onCreate(savedInstanceState);

		Client.init(this.getApplicationContext(),
				xor_secret("S)TEPoEmjI_]" + ((char) 127)
						+ "Zf]1C(}INbRP:n>O\\7C1uo|okrl@=BjUXX#8M;bQG:5"), null);

		final TermsOfServiceController controller = new TermsOfServiceController(
				new TermsOfServiceControllerObserver() {
					@Override
					public void termsOfServiceControllerDidFinish(
							final TermsOfServiceController controller,
							final Boolean accepted) {
						if (accepted != null) {
							// TODO react to this
							if (accepted) {
								updateUserInfo();
							} else {
							}
						}
					}
				});
		controller.query(this);
	}

	/************************************************************************/
	/*																		*/
	/*                  S C O R E L O O P   A C H E I V E M E N T           */
	/*																		*/
	/************************************************************************/

	private static void updateUserInfo() {
		UserController userController = new UserController(
				new RequestControllerObserver() {
					@Override
					public void requestControllerDidReceiveResponse(
							RequestController arg0) {
						mUserCanSubmitScores = !((UserController) arg0)
								.getUser().isAnonymous();
						if (mUserCanSubmitScores) {
							mScoreloopUsername=((UserController) arg0)
									.getUser().getLogin();
							nativeUpdateUserInfo(mScoreloopUsername);
						} else {
							nativeUpdateUserInfo(null);
						}
					}

					@Override
					public void requestControllerDidFail(
							RequestController arg0, Exception arg1) {
						// something's up, not sure what to do.
						nativeUpdateUserInfo("[error]");
					}
				});
		userController.loadUser();
	}

	public static void setUserEmail(String email) {
		class EmailSetter implements Runnable {
			String email;

			public EmailSetter(String email) {
				this.email = email;
			}

			@Override
			public void run() {
				UserController controller = new UserController(
						new RequestControllerObserver() {
							@Override
							public void requestControllerDidFail(
									RequestController arg0, Exception arg1) {
								AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
										mSingleton);
								alertDialogBuilder
										.setTitle("Email in use")
										.setMessage(
												"Check your email to confirm this email is yours. Look for a message from Scoreloop. Once you merge the devices, restart Tux Racer to use the alias associated with the new email. Open browser now?")
										.setCancelable(false)
										.setPositiveButton(
												"Yes",
												new DialogInterface.OnClickListener() {
													public void onClick(
															DialogInterface dialog,
															int id) {
														Uri uri = Uri
																.parse("http://www.google.com");
														Intent intent = new Intent(
																Intent.ACTION_VIEW,
																uri);
														mSingleton
																.startActivity(intent);
													}
												})
										.setNegativeButton(
												"No",
												new DialogInterface.OnClickListener() {
													public void onClick(
															DialogInterface dialog,
															int id) {
													}
												}).create().show();
							}

							@Override
							public void requestControllerDidReceiveResponse(
									RequestController arg0) {
								AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
										mSingleton);
								alertDialogBuilder
										.setTitle("Success")
										.setMessage(
												"Email address was set successfully. Now you can reuse your alias or use one you set previously on another device.")
										.setCancelable(false)
										.setPositiveButton(
												"OK",
												new DialogInterface.OnClickListener() {
													public void onClick(
															DialogInterface dialog,
															int id) {
													}
												}).create().show();
								updateUserInfo();
							}
						});
				controller.setUser(Session.getCurrentSession().getUser());
				controller.getUser().setEmailAddress(email);
				controller.submitUser();
			}
		}
		if (android.util.Patterns.EMAIL_ADDRESS.matcher(email).matches()) {
			EmailSetter setter = new EmailSetter(email);
			mSingleton.runOnUiThread(setter);
		} else {
			mSingleton.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
							mSingleton);

					alertDialogBuilder
							.setTitle("Invalid Email")
							.setMessage("That email seems to be invalid.")
							.setCancelable(false)
							.setPositiveButton("OK",
									new DialogInterface.OnClickListener() {
										public void onClick(
												DialogInterface dialog, int id) {
										}
									}).create().show();
				}
			});
		}
	}

	public static void setUserAlias(String alias) {
		class AliasSetter implements Runnable {
			String alias;

			public AliasSetter(String alias) {
				this.alias = alias;
			}

			@Override
			public void run() {
				UserController controller = new UserController(
						new RequestControllerObserver() {
							@Override
							public void requestControllerDidFail(
									RequestController arg0, Exception arg1) {
								AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
										mSingleton);
								alertDialogBuilder
										.setTitle("Alias in use")
										.setMessage(
												"Try again or (if you are using this alias on another device) set your email here and then check for a message from Scoreloop asking if you want to merge the devices")
										.setPositiveButton(
												"OK",
												new DialogInterface.OnClickListener() {
													public void onClick(
															DialogInterface dialog,
															int id) {
													}
												}).create().show();
							}

							@Override
							public void requestControllerDidReceiveResponse(
									RequestController arg0) {
								AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
										mSingleton);
								alertDialogBuilder
										.setTitle("Success")
										.setMessage("Now go set a high score!")
										.setPositiveButton(
												"OK",
												new DialogInterface.OnClickListener() {
													public void onClick(
															DialogInterface dialog,
															int id) {
													}
												}).create().show();
								updateUserInfo();
							}
						});
				controller.setUser(Session.getCurrentSession().getUser());
				controller.getUser().setLogin(alias);
				controller.submitUser();
				Log.d("scoreloop", "setting username to " + alias);
			}
		}
		AliasSetter aliasSetter = new AliasSetter(alias);
		mSingleton.runOnUiThread(aliasSetter);
	}

	public static void displayTextInputDialog(String title, String message, boolean isUsername) {
		Log.d("dialog", "displaying dialog");
		class DialogDisplayer implements Runnable {
			String title, message;
			boolean isUsername;
			EditText input;
			
			public DialogDisplayer(String title, String message, boolean isUsername) {
				this.title = title;
				this.message = message;
				this.isUsername = isUsername;
			}

			@Override
			public void run() {
				AlertDialog.Builder alert = new AlertDialog.Builder(mSingleton);
				alert.setTitle(title);
				alert.setMessage(message);
				input = new EditText(mSingleton);
				if (isUsername)
				{
					input.setText(mScoreloopUsername);
					input.setSelection(mScoreloopUsername.length());
				}
				alert.setView(input);

				alert.setPositiveButton("OK",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,
									int whichButton) {
								nativeTextCallback(input.getText().toString());
							}
						});

				alert.setNegativeButton("Cancel",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,
									int whichButton) {
							}
						});

				alert.show();
			}
		}
		DialogDisplayer displayer = new DialogDisplayer(title, message, isUsername);
		mSingleton.runOnUiThread(displayer);
	}

	public static void requestScores(int scoreMode) {
		class ScoreRequester implements Runnable {
			public int scoreMode;

			@Override
			public void run() {
				class ScoreboardObserver implements RequestControllerObserver {
					int scoreMode;
					int expectedCallbacks;
					int callbacks = 0;
					HashSet<Score> scores = new HashSet<Score>();

					public ScoreboardObserver(int expectedCallbacks,
							int scoreMode) {
						this.expectedCallbacks = expectedCallbacks;
						this.scoreMode = scoreMode;
					}

					@Override
					public void requestControllerDidFail(
							RequestController controller, Exception ex) {
						ex.printStackTrace();
						callbacks++;
						if (callbacks == expectedCallbacks) {
							process();
						}
					}

					@Override
					public void requestControllerDidReceiveResponse(
							RequestController controller) {
						callbacks++;
						scores.addAll(((ScoresController) controller)
								.getScores());
						if (callbacks == expectedCallbacks) {
							process();
						}
					}

					private void process() {
						ArrayList<Score> scoreList = new ArrayList<Score>(
								scores);
						Collections.sort(scoreList, new Comparator<Score>() {
							public int compare(Score a, Score b) {
								return a.getRank() - b.getRank();
							}
						});
						// if there are >10 unique elements, the user isn't in
						// the top 10 so we remove the 10th element (index 9)
						// and allow the user (previously position 11/index 10)
						// to take his or her spot.
						if (scoreList.size() > 10) {
							scoreList.remove(9);
						}
						ArrayList<String> scoreStrings = new ArrayList<String>();
						for (Score score : scoreList) {
							scoreStrings.add(score.getRank().toString() + "\t"
									+ score.getUser().getDisplayName() + "\t"
									+ score.getResult().intValue());
						}
						nativeScoreloopGotScores(scoreMode,
								scoreStrings.toArray());
					}
				}
				;
				Log.d("scores", "Starting score acquisition.");
				ScoreboardObserver observer = new ScoreboardObserver(2,
						scoreMode);
				ScoresController scoresController = new ScoresController(
						Session.getCurrentSession(), observer);
				scoresController.setMode(scoreMode);
				scoresController.setRangeLength(10);
				scoresController.loadRangeAtRank(1);
				scoresController = new ScoresController(
						Session.getCurrentSession(), observer);
				scoresController.setMode(scoreMode);
				scoresController.setRangeLength(1);
				scoresController.loadRangeForUser(Session.getCurrentSession()
						.getUser());
			}
		}
		;
		ScoreRequester requester = new ScoreRequester();
		requester.scoreMode = scoreMode;
		mSingleton.runOnUiThread(requester);
	}

	// this is a really ugly method, but I need to chain several callbacks
	// together and I don't see a way to do that without duplicating code
	public static void promptForAlias(int scoreMode, int scoreValue) {
		class AliasPromptAndSubmit implements Runnable {
			int scoreMode;
			int scoreValue;
			String title;
			String message;

			public AliasPromptAndSubmit(int scoreMode, int scoreValue,
					String title, String message) {
				this.scoreMode = scoreMode;
				this.scoreValue = scoreValue;
				this.title = title;
				this.message = message;
			}

			@Override
			public void run() {
				AlertDialog.Builder alert = new AlertDialog.Builder(mSingleton);
				alert.setTitle("Submit Score?");
				alert.setMessage("Would you like to submit your score? You will need to set your alias first, which will publicly appear next to your scores. You can do this at any time in the Settings menu.");
				alert.setPositiveButton("OK",
						new DialogInterface.OnClickListener() {
							private void setAlias() {
								AlertDialog.Builder alert = new AlertDialog.Builder(
										mSingleton);
								alert.setTitle(title);
								alert.setMessage(message);
								final EditText input = new EditText(mSingleton);
								alert.setView(input);
								alert.setPositiveButton("OK",
										new DialogInterface.OnClickListener() {
											public void onClick(
													DialogInterface dialog,
													int whichButton) {
												UserController controller = new UserController(
														new RequestControllerObserver() {
															@Override
															public void requestControllerDidFail(
																	RequestController arg0,
																	Exception arg1) {
																AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
																		mSingleton);
																alertDialogBuilder
																		.setTitle(
																				"Alias in use")
																		.setMessage(
																				"If you are using this alias on another device you will need to set your email address in the Settings menu. Otherwise, you can try again with a different alias.")
																		.setPositiveButton(
																				"Try Again",
																				new DialogInterface.OnClickListener() {
																					public void onClick(
																							DialogInterface dialog,
																							int id) {
																						setAlias(); // do
																									// this
																									// recursively
																					}
																				})
																		.setNegativeButton(
																				"Cancel",
																				new DialogInterface.OnClickListener() {
																					public void onClick(
																							DialogInterface dialog,
																							int id) {
																					}
																				})
																		.create()
																		.show();
															}

															@Override
															public void requestControllerDidReceiveResponse(
																	RequestController arg0) {
																AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
																		mSingleton);
																alertDialogBuilder
																		.setTitle(
																				"Success!")
																		.setMessage(
																				"Submitting your score.")
																		.setPositiveButton(
																				"OK",
																				new DialogInterface.OnClickListener() {
																					public void onClick(
																							DialogInterface dialog,
																							int id) {
																					}
																				})
																		.create()
																		.show();
																mUserCanSubmitScores = true;
																updateUserInfo();
																submitScore(
																		scoreMode,
																		scoreValue);
															}
														});
												controller.setUser(Session
														.getCurrentSession()
														.getUser());
												controller.getUser().setLogin(
														input.getText()
																.toString());
												controller.submitUser();
											}
										});

								alert.setNegativeButton("Cancel",
										new DialogInterface.OnClickListener() {
											public void onClick(
													DialogInterface dialog,
													int whichButton) {
											}
										});
								alert.show();
							}

							public void onClick(DialogInterface dialog,
									int whichButton) {
								setAlias();
							}
						});

				alert.setNegativeButton("Cancel",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,
									int whichButton) {
							}
						});

				alert.setNeutralButton("Never Prompt",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,
									int whichButton) {
								nativeDisableAliasPrompt();
							}
						}).create().show();
			}
		}
		AliasPromptAndSubmit submitter = new AliasPromptAndSubmit(
				scoreMode,
				scoreValue,
				"Enter Alias",
				"Enter the alias you want to appear next to your scores. Note that you will not be able to reuse this on other devices unless you also enter an email in the Settings menu.");
		mSingleton.runOnUiThread(submitter);
	}

	public static boolean submitScore(int scoreMode, int scoreValue) {
		class ScoreSubmitter implements Runnable {
			int mode;
			double scoreValue;

			@Override
			public void run() {
				final Score score = new Score(scoreValue, null);
				score.setMode(mode);
				final ScoreController scoreController = new ScoreController(
						new RequestControllerObserver() {
							@Override
							public void requestControllerDidFail(
									RequestController arg0, Exception arg1) {
								Log.d("scores", "score submission failed");
							}

							@Override
							public void requestControllerDidReceiveResponse(
									RequestController arg0) {
								Log.d("scores", "score submission succeeded");
							}
						});
				scoreController.submitScore(score);
			}
		}
		ScoreSubmitter submitter = new ScoreSubmitter();
		submitter.mode = scoreMode;
		submitter.scoreValue = scoreValue;
		if (mUserCanSubmitScores) {
			mSingleton.runOnUiThread(submitter);
			return true;
		} else {
			return false;
		}
	}
}
